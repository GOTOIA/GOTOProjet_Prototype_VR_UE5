# Prototype_VR_V1

Developed with Unreal Engine 5


#MAC Version

# Unreal Engine 5 – macOS / Xcode Compatibility Fix

## Contexte

Ce projet Unreal Engine (UE 5.5.x) est développé sur **macOS (Apple Silicon)** avec un projet **C++**.

Lors de l’ouverture du projet via le fichier `.uproject`, Unreal Engine peut afficher les erreurs suivantes :

- `Missing <ProjectName> Modules`
- `Platform Mac is not a valid platform to build`
- Échec de compilation lors du rebuild automatique

Ces erreurs peuvent apparaître **même sur un projet créé localement sur Mac**, avec une installation Unreal Engine fonctionnelle.

---

## Cause réelle du problème

Le problème ne vient **ni du projet**, ni des plateformes Unreal, mais d’une **incompatibilité entre Unreal Engine et la version installée de Xcode**.

Unreal Engine 5.5.x impose une **version maximale de Xcode** autorisée via UnrealBuildTool (UBT).  
Si la version de Xcode installée est **plus récente que la version maximale déclarée**, alors :

- UBT invalide silencieusement la plateforme **Mac**
- La compilation est bloquée
- Le message d’erreur retourné est trompeur

---

## Correctif appliqué

Le correctif consiste à **mettre à jour la version maximale de Xcode autorisée** dans le fichier de configuration interne du moteur.

location : users/shared/Epic Games/UE_5.5/Engine/Config/Apple/Apple_SDK.json

### Fichier modifié
location : users/shared/Epic Games/UE_5.5/Engine/Config/Apple/Apple_SDK.json


### Modifications effectuées

1. Mise à jour de la version maximale de Xcode :

```json
"MaxVersion": "26.9.0"

#Compilation manuelle UE 

"/Users/Shared/Epic Games/UE_5.5/Engine/Build/BatchFiles/Mac/Build.sh" \
POC_VREditor Mac Development \
-project="$(pwd)/POC_VR.uproject" \
-waitmutex -progress

#Debug Editor
Build.sh POC_VREditor Mac Debug -project=".../POC_VR.uproject"

#Build Shipping (jeu final)

Build.sh POC_VR Mac Shipping -project=".../POC_VR.uproject"

#Nettoyage avant recompilation (recommandé en cas d’erreur)
rm -rf Binaries Intermediate

#Logs de compilation
~/Library/Application Support/Epic/UnrealBuildTool/Log.txt





