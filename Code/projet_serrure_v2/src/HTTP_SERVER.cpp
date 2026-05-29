#include <http_server.h>

#define IMG_WIDTH 320
#define IMG_HEIGHT 240
#define IMG_PIXELS (IMG_WIDTH * IMG_HEIGHT)
#define PIXEL_DIFF_THRESHOLD 80  // seuil pour considérer qu'un pixel a changé
#define ZONE_COLS 5              // nombre de colonnes pour détection par zone
#define ZONE_ROWS 5              // nombre de lignes pour détection par zone
#define ZONE_PIXEL_THRESHOLD 270 // pixels changés dans une zone pour la considérer active
#define HUMAN_ZONE_COUNT 10      // nombre de zones centrales actives pour déclencher détection

// --- VARIABLES GLOBALES ---
httpd_handle_t camera_httpd = NULL;
uint8_t *gray_curr = nullptr;
uint8_t *gray_prev = nullptr;
uint8_t *last_jpeg = nullptr;
size_t last_jpeg_len = 0;

SemaphoreHandle_t jpeg_mutex = NULL;
SemaphoreHandle_t cam_mutex = NULL;
bool first_frame = true;
uint32_t dernier_dection = 0;
bool humain_detecte = false;

// --- INIT DES BUFFERS ---
void init_buffers()
{
    gray_curr = (uint8_t *)ps_malloc(IMG_PIXELS);
    gray_prev = (uint8_t *)ps_malloc(IMG_PIXELS);
    jpeg_mutex = xSemaphoreCreateMutex();
    cam_mutex = xSemaphoreCreateMutex();

    if (!gray_curr || !gray_prev || !jpeg_mutex || !cam_mutex)
    {
        Serial.println("Erreur d'allocation mémoire !");
        while (1)
            ;
    }
}

// --- CONVERSION CAMERA FB -> GRIS ---
void fb_to_gray(camera_fb_t *fb, uint8_t *gray_buf)
{
    if (!fb || !gray_buf)
        return;

    for (int i = 0; i < IMG_PIXELS; i++)
    {
        uint8_t r = fb->buf[i * 3 + 0];
        uint8_t g = fb->buf[i * 3 + 1];
        uint8_t b = fb->buf[i * 3 + 2];
        gray_buf[i] = (r * 30 + g * 59 + b * 11) / 100;
    }
}

// --- DETECTION HUMAINE PAR ZONE ---
bool detect_human_by_zone(uint8_t *curr, uint8_t *prev)
{
    int zone_width = IMG_WIDTH / ZONE_COLS;
    int zone_height = IMG_HEIGHT / ZONE_ROWS;
    int active_zones = 0;

    for (int zy = 0; zy < ZONE_ROWS; zy++)
    {
        for (int zx = 0; zx < ZONE_COLS; zx++)
        {
            // on ignore les zones trop à gauche ou droite
            if (zx == 0 || zx == ZONE_COLS - 1)
                continue;

            int diff_count = 0;
            for (int y = zy * zone_height; y < (zy + 1) * zone_height; y += 2)
            {
                for (int x = zx * zone_width; x < (zx + 1) * zone_width; x += 2)
                {
                    int idx = y * IMG_WIDTH + x;
                    if (abs(curr[idx] - prev[idx]) > PIXEL_DIFF_THRESHOLD)
                        diff_count++;
                }
            }

            if (diff_count > ZONE_PIXEL_THRESHOLD)
                active_zones++;
        }
    }

    return (active_zones >= HUMAN_ZONE_COUNT);
}

// --- TASK CAMERA ---
void camera_task(void *pvParameters)
{
    while (true)
    {
        camera_fb_t *fb = nullptr;

        if (xSemaphoreTake(cam_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            fb = esp_camera_fb_get();
            xSemaphoreGive(cam_mutex);
        }

        if (!fb)
        {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // --- SAUVEGARDE JPEG ---
        if (xSemaphoreTake(jpeg_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            if (last_jpeg == nullptr || last_jpeg_len < fb->len)
            {
                if (last_jpeg)
                    free(last_jpeg);
                last_jpeg = (uint8_t *)ps_malloc(fb->len);
            }
            if (last_jpeg)
            {
                memcpy(last_jpeg, fb->buf, fb->len);
                last_jpeg_len = fb->len;
            }
            xSemaphoreGive(jpeg_mutex);
        }

        // --- CONVERSION GRIS ---
        fb_to_gray(fb, gray_curr);

        // --- DETECTION HUMAINE ---
        bool humanNow = !first_frame && detect_human_by_zone(gray_curr, gray_prev);

        if (humanNow)
        {
            if (!humain_detecte)
            {
                Envoi_MQTT("esp32/capteur/mouvement", "ON", true);
                humain_detecte = true;
            }
            dernier_dection = millis();
        }
        else
        {
            if (humain_detecte)
            {
                if ((millis() - dernier_dection) >= 30000)
                {
                    humain_detecte = false;
                    Envoi_MQTT("esp32/capteur/mouvement", "OFF", true);
                }
            }
        }

        memcpy(gray_prev, gray_curr, IMG_PIXELS);
        first_frame = false;

        esp_camera_fb_return(fb);
        vTaskDelay(pdMS_TO_TICKS(50)); // ~20 FPS
    }
}

// --- STREAM MJPEG ---
static esp_err_t stream_handler(httpd_req_t *req)
{
    static const char *BOUNDARY = "frame";
    char buf[128];

    httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    httpd_resp_set_hdr(req, "Connection", "close");

    while (true)
    {
        camera_fb_t *fb = nullptr;

        if (xSemaphoreTake(cam_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            fb = esp_camera_fb_get();
            xSemaphoreGive(cam_mutex);
        }

        if (!fb)
            return ESP_FAIL;

        size_t hlen = snprintf(buf, sizeof(buf),
                               "--%s\r\n"
                               "Content-Type: image/jpeg\r\n"
                               "Content-Length: %u\r\n\r\n",
                               BOUNDARY, fb->len);

        if (httpd_resp_send_chunk(req, buf, hlen) != ESP_OK ||
            httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len) != ESP_OK ||
            httpd_resp_send_chunk(req, "\r\n", 2) != ESP_OK)
        {
            esp_camera_fb_return(fb);
            break;
        }

        esp_camera_fb_return(fb);
        vTaskDelay(pdMS_TO_TICKS(30));
    }

    return ESP_OK;
}

// --- HANDLER: Servir une image depuis SPIFFS ---
// URL: /image?f=/img_20260514_143022.jpg
static esp_err_t image_handler(httpd_req_t *req)
{
    char query[128];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
    {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char param[64];
    if (httpd_query_key_value(query, "f", param, sizeof(param)) != ESP_OK)
    {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // S'assurer que le chemin commence par /
    String path = String(param).startsWith("/") ? String(param) : "/" + String(param);

    File file = SPIFFS.open(path, FILE_READ);
    if (!file)
    {
        Serial.printf("Image non trouvée: %s\n", path.c_str());
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    size_t fileSize = file.size();
    uint8_t *buf = (uint8_t *)malloc(fileSize);
    if (!buf)
    {
        file.close();
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    file.read(buf, fileSize);
    file.close();

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    esp_err_t res = httpd_resp_send(req, (const char *)buf, fileSize);

    free(buf);
    return res;
}

// --- HANDLER: Dernière image capturée ---
// URL: /last
static esp_err_t last_image_handler(httpd_req_t *req)
{
    String newestName = "";
    String newestTs   = "00000000_000000";

    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while (file)
    {
        String name = String(file.name());
        // Normaliser: ajouter / si absent
        String fullName = name.startsWith("/") ? name : "/" + name;

        if (fullName.startsWith("/img_") && fullName.endsWith(".jpg"))
        {
            String ts = fullName.substring(5, 20); // extraire YYYYMMDD_HHMMSS
            if (ts > newestTs)
            {
                newestTs   = ts;
                newestName = fullName;
            }
        }
        file = root.openNextFile();
    }

    if (newestName == "")
    {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // Redirection vers /image?f=<nom>
    char redirect[64];
    snprintf(redirect, sizeof(redirect), "/image?f=%s", newestName.c_str());
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", redirect);
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// --- HANDLER: Liste de toutes les images en JSON ---
// URL: /images
static esp_err_t list_images_handler(httpd_req_t *req)
{
    String ip = WiFi.localIP().toString();
    String json = "[";
    bool first = true;

    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while (file)
    {
        String name = String(file.name());
        // Normaliser: ajouter / si absent
        String fullName = name.startsWith("/") ? name : "/" + name;

        if (fullName.startsWith("/img_") && fullName.endsWith(".jpg"))
        {
            if (!first) json += ",";
            json += "{";
            json += "\"name\":\"" + fullName + "\",";
            json += "\"url\":\"http://" + ip + ":8026/image?f=" + fullName + "\",";
            json += "\"size\":" + String(file.size());
            json += "}";
            first = false;
        }
        file = root.openNextFile();
    }

    json += "]";

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

// --- HANDLER: Galerie HTML des 20 dernières images ---
// URL: /gallery
static esp_err_t gallery_handler(httpd_req_t *req)
{
    String ip = WiFi.localIP().toString();

    // Collecter les images triées (plus récente en premier)
    String names[20];
    int count = 0;

    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while (file && count < 20)
    {
        String name = String(file.name());
        String fullName = name.startsWith("/") ? name : "/" + name;
        if (fullName.startsWith("/img_") && fullName.endsWith(".jpg"))
        {
            names[count++] = fullName;
        }
        file = root.openNextFile();
    }

    // Tri décroissant (plus récente en premier) — tri à bulles simple
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (names[j] < names[j + 1])
            {
                String tmp = names[j];
                names[j] = names[j + 1];
                names[j + 1] = tmp;
            }

    // Construire la page HTML
    String html = R"(<!DOCTYPE html>
<html lang='fr'>
<head>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<title>ESP32 Galerie</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { background: #111; color: #eee; font-family: sans-serif; padding: 12px; }
  h1 { text-align: center; font-size: 1.1em; margin-bottom: 12px; color: #4fc3f7; }
  .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(140px, 1fr)); gap: 8px; }
  .card { background: #1e1e1e; border-radius: 8px; overflow: hidden; cursor: pointer; transition: transform 0.2s; }
  .card:hover { transform: scale(1.03); }
  .card img { width: 100%; display: block; }
  .card .ts { font-size: 0.65em; text-align: center; padding: 4px; color: #aaa; }
  .modal { display:none; position:fixed; top:0; left:0; width:100%; height:100%;
           background:rgba(0,0,0,0.9); z-index:99; justify-content:center; align-items:center; flex-direction:column; }
  .modal.open { display:flex; }
  .modal img { max-width:95%; max-height:85vh; border-radius:8px; }
  .modal .info { margin-top:10px; font-size:0.8em; color:#aaa; }
  .modal .close { position:absolute; top:16px; right:20px; font-size:2em; cursor:pointer; color:#fff; }
</style>
</head>
<body>
<h1>📷 ESP32 — 20 dernières captures</h1>
<div class='grid' id='grid'></div>
<div class='modal' id='modal'>
  <span class='close' onclick='closeModal()'>✕</span>
  <img id='modal-img' src='' alt=''>
  <div class='info' id='modal-info'></div>
</div>
<script>
const images = [)";

    for (int i = 0; i < count; i++)
    {
        // Extraire timestamp lisible depuis le nom: /img_20260515_003903.jpg
        String n = names[i]; // /img_20260515_003903.jpg
        String ts = "";
        if (n.length() >= 20) {
            // YYYYMMDD_HHMMSS
            String d = n.substring(5, 13); // 20260515
            String t = n.substring(14, 20); // 003903
            ts = d.substring(6,8) + "/" + d.substring(4,6) + "/" + d.substring(0,4)
               + " " + t.substring(0,2) + "h" + t.substring(2,4) + "m" + t.substring(4,6) + "s";
        }
        html += "{\"url\":\"http://" + ip + ":8026/image?f=" + n + "\",\"ts\":\"" + ts + "\"}";
        if (i < count - 1) html += ",";
    }

    html += R"(];
const grid = document.getElementById('grid');
images.forEach((img, i) => {
  const card = document.createElement('div');
  card.className = 'card';
  card.innerHTML = `<img src="${img.url}" loading="lazy" alt="capture ${i+1}">
                    <div class="ts">${img.ts}</div>`;
  card.onclick = () => openModal(img.url, img.ts);
  grid.appendChild(card);
});
function openModal(url, ts) {
  document.getElementById('modal-img').src = url;
  document.getElementById('modal-info').textContent = ts;
  document.getElementById('modal').classList.add('open');
}
function closeModal() {
  document.getElementById('modal').classList.remove('open');
}
document.getElementById('modal').addEventListener('click', function(e) {
  if (e.target === this) closeModal();
});
// Rafraîchissement auto toutes les 30s
setTimeout(() => location.reload(), 30000);
</script>
</body>
</html>)";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_send(req, html.c_str(), html.length());
    return ESP_OK;
}

// --- START SERVER ---
void startCameraServer()
{
    init_buffers();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 8026;
    config.max_open_sockets = 5;
    config.stack_size = 8192;

    httpd_uri_t stream_uri     = {"/stream",  HTTP_GET, stream_handler,      NULL};
    httpd_uri_t image_uri      = {"/image",   HTTP_GET, image_handler,       NULL};
    httpd_uri_t last_uri       = {"/last",    HTTP_GET, last_image_handler,  NULL};
    httpd_uri_t list_uri       = {"/images",  HTTP_GET, list_images_handler, NULL};
    httpd_uri_t gallery_uri    = {"/gallery", HTTP_GET, gallery_handler,     NULL};

    if (httpd_start(&camera_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(camera_httpd, &stream_uri);
        httpd_register_uri_handler(camera_httpd, &image_uri);
        httpd_register_uri_handler(camera_httpd, &last_uri);
        httpd_register_uri_handler(camera_httpd, &list_uri);
        httpd_register_uri_handler(camera_httpd, &gallery_uri);

        Serial.println("✅ Serveur caméra prêt");
        Serial.printf("📷 Stream:        http://%s:8026/stream\n",  WiFi.localIP().toString().c_str());
        Serial.printf("🖼️  Dernière img:  http://%s:8026/last\n",   WiFi.localIP().toString().c_str());
        Serial.printf("📂 Liste images:  http://%s:8026/images\n",  WiFi.localIP().toString().c_str());
        Serial.printf("🖼️  Galerie:       http://%s:8026/gallery\n", WiFi.localIP().toString().c_str());
    }

    xTaskCreatePinnedToCore(
        camera_task,
        "camera_task",
        6000,
        NULL,
        1,
        NULL,
        1);
}

// --- ENVOYER UNE IMAGE VERS HOME ASSISTANT ---
bool sendImageToHA(const char *filepath)
{
    File file = SPIFFS.open(filepath, FILE_READ);
    if (!file)
    {
        Serial.printf("Erreur ouverture: %s\n", filepath);
        return false;
    }

    size_t fileSize = file.size();
    uint8_t *buf = (uint8_t *)malloc(fileSize);
    if (!buf)
    {
        Serial.println("Erreur: malloc échoué");
        file.close();
        return false;
    }

    file.read(buf, fileSize);
    file.close();

    HTTPClient http;
    char url[128];
    snprintf(url, sizeof(url), "http://" HA_IP ":%d/api/webhook/esp32_image", HA_PORT);

    Serial.printf("→ Envoi vers: %s\n", url);

    http.begin(url);
    http.setTimeout(10000);
    http.addHeader("Content-Type", "image/jpeg");
    http.addHeader("Authorization", "Bearer " HA_TOKEN);
    http.addHeader("X-Filename", filepath);

    int httpCode = http.POST(buf, fileSize);
    bool success = (httpCode == 200 || httpCode == 201 || httpCode == 204);

    if (success)
    {
        Serial.printf("✓ Envoyé: %s (%d bytes)\n", filepath, fileSize);
    }
    else
    {
        Serial.printf("✗ Erreur HTTP: %d → %s\n", httpCode, http.errorToString(httpCode).c_str());
    }

    http.end();
    free(buf);
    return success;
}

// --- ENVOYER TOUTES LES IMAGES SPIFFS VERS HA ---
void sendAllImagesToHA()
{
    File root = SPIFFS.open("/");
    if (!root || !root.isDirectory())
    {
        Serial.println("Erreur: impossible d'ouvrir SPIFFS");
        return;
    }

    File file = root.openNextFile();
    int count = 0;

    while (file)
    {
        String name = String(file.name());
        String path = name.startsWith("/") ? name : "/" + name;

        if (name.endsWith(".jpg") || name.endsWith(".jpeg") ||
            name.endsWith(".JPG") || name.endsWith(".JPEG"))
        {
            Serial.printf("SPIFFS: envoi %s (%u octets)\n", name.c_str(), file.size());
            if (sendImageToHA(path.c_str()))
                count++;
            delay(10);
        }
        file = root.openNextFile();
    }

    Serial.printf("✓ %d images envoyées à Home Assistant\n", count);
}