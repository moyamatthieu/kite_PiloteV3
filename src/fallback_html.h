#ifndef FALLBACK_HTML_H
#define FALLBACK_HTML_H

const char* fallbackHtml = R"(
<html>
  <head>
    <title>Kite PiloteV3 - Serveur Web</title>
  </head>
  <body>
    <h1>Kite PiloteV3 - Serveur Web</h1>
    <p>Mode génération de code HTML activé car SPIFFS est indisponible.</p>
    <p>État du système : %s</p>
    <p>Adresse IP : %s</p>
  </body>
</html>
)";

#endif  // FALLBACK_HTML_H
