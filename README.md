IW
==

interactive window


Vitrine interactive Xone écrit en C++ :

Installation sur windows 7 ou 8 :

        Visual studio express 2012 for desktop
        http://libcinder.org/ pour visual 2012

Testé avec la ASUS XTION Pro et utilise le middleware Openni. Les blocks suivants sont utilisés : openCv, Openni, Warping, OSC inclut dans le dépot

Pour compiler les projets mette une variable d'environnement CINDER_085 pointant sur le le dossier cinder 2012.

2 applications communiquant en OSC : User2OSC et Xone
User2OSC se trouve dans le répertoire XOne/blocks/openni/sample
User2OSC permet de détecter des personnes (avec le middleware OpenNi) et de communiquer son centre de gravité et ses mains par OSC

Touches : Pour le warping appuyer w : voir le fichier help_warping.png à la racine touche i : pour afficher infos et pour régler tous les paramètres touche f : pour changer le fullscreen touche escape : pour quitter

Fichier json d'initilialisation des paramètres :

voir fichier init.json dans le dossier asset.

{

    "config" : {

        "visu" : {

            "mScaleLogoX" : 0.40, // scale en x du texte "mScaleLogoY" : 0.91, // scale en y du texte "mPosText" : {

                "x" : 0.0, "y" : -100.0, "z" : 0.0

            }, "mTranslaleAllObj" : { // translation de toutes les boules Xones

                "x" : 0.03, "y" : 0.06

            }, "mTranslate1" : { // translation de la boule 1

                "x" : -0.980, "y" : -0.226

            }, "mTranslate2" : { // translation de la boule 2

                "x" : -0.504, "y" : -0.230

            }, "mTranslate3" : { // translation de la boule 3

                "x" : -0.03, "y" : -0.222

            }, "mTranslate4" : { // translation de la boule 4

                "x" : 0.448, "y" : -0.222

            }, "mTranslate5" : { // translation de la boule 5

                "x" : 0.922, "y" : -0.226

            }, "mTranslateZ" : 0.0, "mScale1" : 0.104, // scale de la boule 1 "mScale2" : 0.104, // scale de la boule 2 "mScale3" : 0.104, // scale de la boule 3 "mScale4" : 0.104, // scale de la boule 4 "mScale5" : 0.104, // scale de la boule 5 "mSaveParam" : true, // ne fonctionne pas "mDisplayInfos" : false, // pour afficher les infos de debug ou de réglage "mAngleInitObj" : { // réglage de l'angle initial des boules 3d en degré

                "x" : -45.0, "y" : 45.0, "z" : 0.0

            }

        }, "interaction" : {

            "mOffset" : { // offset angulaire des boules 3d
                "x" : 0.0, "y" : 0.0

            }, "mFactor" : { // facteur d'amplification de la rotation des boules

                "x" : 10.0, "y" : -1.0

            }, "mOffsetControl" : { // offset angulaire des boules 3d par controle avec la main

                "x" : 0.0, "y" : 0.0

            }, "mFactorControl" : { // facteur d'amplification de la rotation des boules par controle avec la main

                "x" : 20.0, "y" : -20.0

            }, "mCubeInteraction" : { // cube d'interaction pour prendre le contrôle avec a main

                "x" : 300.0, "y" : 400.0, "z" : 500.0

            }, "mOffsetLimitScreenZ" : 1500.0, "mMoyDetectMovementProjectiveX" : 15.0, // moyenne sur 15 éléments "mMoyDetectMovementZ" : 120.0, "mZLimit" : 2600.0, // limite de la détection "mDefautRotation" : {

                "x" : 0.0, "y" : 10.0

            }

        }

    }

}
