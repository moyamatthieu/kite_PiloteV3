# Éditeur de diagramme dans VS Code

L'éditeur visuel de diagramme dans Wokwi pour VS Code vous permet de modifier le diagramme de votre projet de simulation. Il est disponible dans les forfaits Hobby+ et Pro.

Carte actuellement utilisée: **board-esp32-devkit-c-v4**

## Ouverture de l'éditeur de diagramme

Pour ouvrir l'éditeur de diagramme, cliquez sur un fichier diagram.json dans la vue Explorateur. Le diagramme s'ouvrira dans un nouvel onglet. L'éditeur de diagramme fonctionne également pour les fichiers correspondant au modèle diagram.*.json, comme diagram.esp32.json. Cela est utile si vous avez plusieurs cartes cibles dans votre projet et que vous souhaitez maintenir un diagramme pour chaque carte cible.

Si vous utilisez le forfait Community ou Hobby, vous pourrez visualiser le diagramme, mais pas le modifier dans l'éditeur de diagramme. Vous pouvez toujours modifier le fichier diagram.json dans l'éditeur de texte.

## Édition du diagramme en mode texte

Certaines fonctionnalités avancées ne sont disponibles que si vous modifiez le diagramme dans l'éditeur de texte. Vous pouvez ouvrir l'éditeur de texte en faisant un clic droit sur l'onglet diagram.json, en sélectionnant "Rouvrir l'éditeur avec..." puis en sélectionnant "Éditeur de texte".

## Structure du fichier diagram.json

Le fichier diagram.json contient trois sections principales :
- `version` : version du format (généralement 1)
- `author` : nom de l'auteur ou de l'équipe du projet
- `editor` : indique l'éditeur utilisé (normalement "wokwi")
- `parts` : définit tous les composants utilisés dans le diagramme (ESP32, LCD, boutons, etc.)
- `connections` : décrit les connexions entre les composants
- `dependencies` : liste les dépendances externes (généralement vide)

### Exemple réel de la section parts
Voici un exemple de définition de composant extrait de notre projet :

```json
{
  "type": "wokwi-lcd2004",
  "id": "lcd1",
  "top": -156.8,
  "left": -378.4,
  "attrs": { "pins": "i2c" }
}
```

Chaque composant est défini avec :
- `type` : type de composant (ex: "wokwi-esp32-devkit-v1", "wokwi-lcd2004")
- `id` : identifiant unique du composant (utilisé dans les connexions)
- `top` et `left` : position du composant dans le diagramme
- `attrs` : attributs spécifiques au composant (varient selon le type)
- `rotate` : rotation du composant en degrés (optionnel)

## Création de connexions visibles

Pour assurer que les connexions sont visibles dans le diagramme, suivez ces bonnes pratiques :

1. **Format correct des connexions** : Chaque connexion doit être définie avec le format suivant :
   ```json
   [ "composant1:pin", "composant2:pin", "couleur", [ point1, point2, ... ] ]
   ```

2. **Ordre des composants** : Pour certaines connexions, l'ordre des composants peut affecter la visibilité. Essayez d'inverser l'ordre si une connexion n'est pas visible.

3. **Notation des broches ESP32** : Pour les broches de l'ESP32, utilisez simplement le numéro de GPIO. Notre carte ESP32 DevKit C V4 utilise ce format.
   Exemple correct: `"esp:12"` pour GPIO12
   
   **IMPORTANT**: Vérifiez toujours que les broches dans diagram.json correspondent à celles définies dans config.h.

4. **Nommage des broches des boutons** : Pour les boutons poussoirs, utilisez la notation `btn1:2.l` pour la broche de signal (celle qui va vers l'ESP32) et `btn1:1.l` pour la broche de masse.

5. **Points de trajet optimaux** : Définissez des trajets clairs avec des points bien espacés :
   ```json
   [ "v-19.2", "h430.4", "v-86.4" ]
   ```
   - "v" indique un déplacement vertical (positif = vers le bas)
   - "h" indique un déplacement horizontal (positif = vers la droite)

6. **Conventions de couleur** : Respectez ces standards pour la lisibilité du diagramme:
   - Rouge : alimentation positive (VCC, V+)
   - Noir : masse (GND)
   - Jaune : signaux I2C SCL et signaux analogiques (potentiomètres)
   - Bleu : signaux I2C SDA et bouton bleu
   - Orange : signaux PWM (servo)
   - Vert : bouton vert et certains signaux numériques
   - Purple : signaux du moteur pas à pas
   - Gray : communication série (TX/RX)

### Exemples de connexions réelles de notre projet

#### Connexion des boutons à l'ESP32 (selon config.h actuel) :
```json
[ "btn1:2.l", "esp:27", "blue", [ "v-19.2", "h430.4", "v-86.4" ] ],    // Bouton bleu (BUTTON_BLUE_PIN)
[ "btn2:2.l", "esp:12", "green", [ "v-9.7", "h132.5", "v-67.2" ] ],    // Bouton vert (BUTTON_GREEN_PIN)
[ "btn3:2.l", "esp:14", "yellow", [ "v-19.2", "h423.7", "v-86.4" ] ],  // Bouton jaune (BUTTON_YELLOW_PIN)
[ "btn4:2.l", "esp:26", "red", [ "v-19.2", "h450", "v-86.4" ] ],       // Bouton rouge (BUTTON_RED_PIN)
```

#### Connexion I2C pour l'écran LCD :
```json
[ "lcd1:SDA", "esp:21", "blue", [ "h-28.8", "v-67", "h633.6", "v77.2" ] ],
[ "lcd1:SCL", "esp:22", "yellow", [ "h-38.4", "v-86.1", "h633.6", "v57.9" ] ],
```

#### Connexions des potentiomètres :
```json
[ "pot1:SIG", "esp:34", "yellow", [ "h162.4", "v-883" ] ],       // Potentiomètre direction (POT_DIRECTION)
[ "pot2:SIG", "esp:35", "yellow", [ "h373.6", "v-873.2" ] ],     // Potentiomètre trim (POT_TRIM)
[ "pot3:SIG", "esp:32", "yellow", [ "h38.4", "v-306.3" ] ],      // Potentiomètre longueur (POT_LENGTH)
```

#### Connexion des servomoteurs :
```json
[ "servo1:PWM", "esp:33", "orange", [ "v38.4", "h441.8", "v134.4" ] ],     // Servomoteur trim (SERVO_TRIM_PIN)
[ "servo2:PWM", "esp:25", "orange", [ "v28.8", "h249.8", "v144" ] ],       // Servomoteur direction (SERVO_DIRECTION_PIN)
```

#### Connexion du moteur pas à pas :
```json
[ "stepper1:A+", "esp:5", "purple", [ "v38.4", "h182.4", "v163.2" ] ],     // STEPPER_IN1
[ "stepper1:A-", "esp:18", "blue", [ "v48", "h182.4", "v144" ] ],          // STEPPER_IN2
[ "stepper1:B+", "esp:19", "green", [ "v28.8", "h153.6", "v153.7" ] ],     // STEPPER_IN3
[ "stepper1:B-", "esp:23", "green", [ "v19.2", "h134.39", "v105.6" ] ],    // STEPPER_IN4
```

#### Connexion de la LED indicatrice :
```json
[ "r1:1", "esp:2", "red", [ "v-38.4", "h-57.6" ] ],              // LED via résistance (LED_PIN)
[ "led1:A", "r1:2", "red", [ "v19.2", "h19.2" ] ],               // Anode LED
[ "led1:C", "gnd1:GND", "black", [ "v48", "h-705.2" ] ],         // Cathode LED à GND
```

## Groupement logique des connexions

Pour les projets complexes, il est recommandé de regrouper les connexions par fonctionnalité dans le fichier diagram.json. Même si JSON ne supporte pas les commentaires, vous pouvez organiser les connexions dans cet ordre :

1. Connexions de communication série (TX/RX, moniteur série)
2. Connexions d'interface utilisateur (LCD, LED)
3. Connexions des boutons
4. Connexions des potentiomètres
5. Connexions des servomoteurs
6. Connexions du moteur pas à pas

Cela facilite la maintenance et le débogage du diagramme.

## Validation des connexions

Après avoir modifié le diagramme, vérifiez toujours que :

1. Toutes les broches GPIO utilisées dans diagram.json correspondent exactement aux définitions dans config.h
2. Chaque composant possède toutes ses connexions nécessaires (alimentation, masse, signal)
3. Les chemins de routage ne se croisent pas inutilement
4. Les composants sont positionnés de manière logique et ergonomique

## Conseils pour le dépannage

Si certaines connexions ne sont pas visibles dans le diagramme :
1. Vérifiez l'ordre des composants dans la définition de la connexion
2. Modifiez les points de trajet pour créer un chemin plus direct
3. Utilisez des noms explicites pour les bornes des composants
4. Vérifiez la notation des broches (avec ou sans préfixe "D" ou "GPIO")
5. Vérifiez que les composants référencés existent et que leurs identifiants sont corrects
6. **IMPORTANT**: Assurez-vous que les broches définies dans diagram.json correspondent exactement à celles définies dans config.h

## Types de composants courants

Voici quelques types de composants disponibles dans Wokwi :
- `wokwi-esp32-devkit-v1` : Carte ESP32 DevKit V1
- `wokwi-arduino-uno` : Arduino UNO
- `wokwi-lcd2004` : Écran LCD 20x4
- `wokwi-led` : LED simple
- `wokwi-pushbutton` : Bouton poussoir
- `wokwi-slide-potentiometer` : Potentiomètre linéaire
- `wokwi-servo` : Servomoteur
- `wokwi-stepper-motor` : Moteur pas à pas
- `wokwi-resistor` : Résistance

## Composants spéciaux utilisés dans notre projet

- `board-esp32-devkit-c-v4` : Notre carte ESP32 principale
- `wokwi-lcd2004` avec `pins: "i2c"` : Écran LCD avec interface I2C simplifiée
- `wokwi-pushbutton` : Boutons de navigation avec attribut `color` pour différencier leurs fonctions
- `wokwi-slide-potentiometer` : Potentiomètres pour les contrôles analogiques
- `wokwi-servo` : Servomoteurs pour la direction et le trim
- `wokwi-stepper-motor` : Moteur pas à pas pour le contrôle de la longueur des lignes
- `wokwi-vcc` et `wokwi-gnd` : Sources d'alimentation et de masse

## Exécution de la simulation

Vous pouvez exécuter la simulation en appuyant sur le bouton vert de lecture dans le coin supérieur gauche de l'éditeur. Wokwi ouvrira un nouvel onglet et démarrera la simulation.

## Conseils pour la création de diagrammes complexes

Pour les projets complexes comme notre système de contrôle de kite :
1. Organisez visuellement les composants par fonction (interface utilisateur, actionneurs, capteurs)
2. Utilisez des couleurs cohérentes pour les connexions
3. Espacez suffisamment les composants pour éviter les chevauchements de connexions
4. Documentez les connexions importantes dans votre README ou code source
5. Pensez à scinder les diagrammes très complexes en plusieurs diagram.*.json spécialisés