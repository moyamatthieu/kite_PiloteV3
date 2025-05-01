#ifndef FALLBACK_HTML_H
#define FALLBACK_HTML_H

const char* fallbackHtml = R"(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Kite PiloteV3 - Tableau de bord</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 20px;
        background-color: #f5f5f5;
        color: #333;
      }
      .container {
        max-width: 800px;
        margin: 0 auto;
        background-color: white;
        padding: 20px;
        border-radius: 8px;
        box-shadow: 0 2px 4px rgba(0,0,0,0.1);
      }
      h1 {
        color: #0066cc;
        text-align: center;
        margin-bottom: 20px;
        padding-bottom: 10px;
        border-bottom: 2px solid #eee;
      }
      .info-box {
        background-color: #e9f7fe;
        border-left: 4px solid #0066cc;
        padding: 15px;
        margin-bottom: 15px;
        border-radius: 4px;
      }
      .status {
        font-weight: bold;
      }
      .menu {
        display: flex;
        flex-wrap: wrap;
        gap: 10px;
        margin: 20px 0;
      }
      .btn {
        display: inline-block;
        background-color: #0066cc;
        color: white;
        padding: 10px 15px;
        text-decoration: none;
        border-radius: 4px;
        transition: background-color 0.3s;
      }
      .btn:hover {
        background-color: #004c99;
      }
      .footer {
        text-align: center;
        margin-top: 30px;
        color: #777;
        font-size: 0.8em;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>Kite PiloteV3</h1>
      
      <div class="info-box">
        <p><span class="status">État du système :</span> %s</p>
        <p><span class="status">Adresse IP :</span> %s</p>
        <p><span class="status">Version :</span> 2.0.0</p>
      </div>
      
      <div class="menu">
        <a href="/update" class="btn">Mise à jour OTA</a>
        <a href="/dashboard" class="btn">Tableau de bord</a>
        <a href="/api/info" class="btn">API Info</a>
      </div>
      
      <div class="footer">
        <p>Kite PiloteV3 &copy; 2025 | Système embarqué ESP32</p>
      </div>
    </div>
  </body>
</html>
)";

#endif  // FALLBACK_HTML_H
