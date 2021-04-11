# 1. Projet EL-3032 – 2021 - Simulation réseau de stockage test PCR

## 1.1. Sommaire

- [1. Projet EL-3032 – 2021 - Simulation réseau de stockage test PCR](#1-projet-el-3032--2021---simulation-réseau-de-stockage-test-pcr)
  - [1.1. Sommaire](#11-sommaire)
  - [1.2. Environnement](#12-environnement)
  - [1.3. Compiler l'application](#13-compiler-lapplication)
  - [1.4. Obtenir une application fonctionnel](#14-obtenir-une-application-fonctionnel)
    - [1.4.1. configFile.txt](#141-configfiletxt)

## 1.2. Environnement

L'appplication est utilisable uniquement sur un systeme Linux avec le paquet xterm d'installer comme ce ci : /usr/bin/xterm  

## 1.3. Compiler l'application

Pour compiler l'application il suffit d'executer la commande make dans le réportoire des fichiers sources.  
La commande " make cleanall" peut être utilisé pour nettoyer complétement le répertoire du progamme.  

## 1.4. Obtenir une application fonctionnel

Pour avoir une application fonctionnelle vous aurez besoin d'un minimun de 2 fichiers :  

- configFile.txt
- Un fichier par centre d'archivage

### 1.4.1. configFile.txt

Ce fichier permet de specifier la configuration de chaque centre d'archivage. Il a une structure bien spécifique, 1 ligne est égale à un centre d'archivage