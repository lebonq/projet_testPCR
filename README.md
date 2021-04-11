# 1. Projet EL-3032 – 2021 - Simulation réseau de stockage test PCR

## 1.1. Sommaire

- [1. Projet EL-3032 – 2021 - Simulation réseau de stockage test PCR](#1-projet-el-3032--2021---simulation-réseau-de-stockage-test-pcr)
  - [1.1. Sommaire](#11-sommaire)
  - [1.2. Environnement](#12-environnement)
  - [1.3. Compiler l'application](#13-compiler-lapplication)
  - [1.4. Obtenir une application fonctionnelle](#14-obtenir-une-application-fonctionnelle)
    - [1.4.1. configFile.txt](#141-configfiletxt)
    - [1.4.2. Fichier de résultats](#142-fichier-de-résultats)
  - [1.5. Exécuter l'application](#15-exécuter-lapplication)

## 1.2. Environnement

L'application est utilisable uniquement sur un système Linux avec le paquet xterm d'installer comme ce ci : **/usr/bin/xterm**  

## 1.3. Compiler l'application
 
Pour compiler l'application il suffit d'exécuter la commande **make** dans le répertoire des fichiers sources.  
La commande *make cleanall* peut être utilisée pour nettoyer complètement le répertoire du programme.  

## 1.4. Obtenir une application fonctionnelle

Pour avoir une application fonctionnelle vous aurez besoin d'un minimum de 2 fichiers :  

- configFile.txt
- Un fichier de résultats par centre d'archivage  

Par défaut, le dépôt github contient tous ces fichiers déjà configurés pour 2 centres d'archivage.

### 1.4.1. configFile.txt

Ce fichier permet de spécifier la configuration de chaque centre d'archivage. Il a une structure bien spécifique, 1 ligne est égale à un centre d'archivage.  
Chaque ligne doit contenir le nom du fichier où sont enregistrés les résultats des tests du centre, l'id du centre et le nombre de terminaux ouvert par le centre.  
Par exemple si je veux créer le centre 028, avec 12 terminaux et dont le fichier de stockage des test est resultats_centre_0028.txt, je dois écrire dans le fichier de configuration :  
resultats_centre_0028.txt 0028 12  

### 1.4.2. Fichier de résultats

Un fichier de résultat est très simple a generation. Il suffit d'exécuter **CreationDatabase** en spécifiant l'id du centre et le nombre de tests voulus.  Si je veux générer un fichier avec 50 tests pour le centre 0012 je fais :  
*./CreationDatabase 0012 50*  
Et un fichier resultats_centre_0012.txt sera créé. Avec un taux de 8% de test positif et en moyenne un test fait entre 4 et 48 heures avant le vol.  

## 1.5. Exécuter l'application

Une fois votre fichier configFile.txt complété, pour avoir le programme fonctionnel il faut exécuter **InterArchive**. Il faut spécifier le nom du fichier de configuration, le nombre de centres décrit dans ce fichier ainsi que la taille du buffer. Si je veux exécuter mon application avec le fichier configuration.txt qui contient 4 centre d'archivage et pour lequel je veux un buffer de 50 requêtes je fais :  
*./InterArchive configuration.txt 4 50*
Pour utiliser un terminal il suffit juste de taper le numéro du test que l'on shouaite valider.
