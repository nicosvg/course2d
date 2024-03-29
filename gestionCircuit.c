#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#ifdef __APPLE__
#include <SDL_ttf/SDL_ttf.h>
#else
#include <SDL/SDL_ttf.h>
#endif
#include "menu.h"
#include "gestionCircuit.h"
#include "gestionFichiers.h"
#include "physique.h"
#include "affichage.h"



int allocationVoiture (SDL_Surface ***surface,  Voiture *voiture) {
	int i,n;
	int nbImages=32;
	int largeur=96,hauteur=96;
	for (i=0;i<32;i++){
		voiture->cheminImage[13]=i-(i/10)*10+48;
		voiture->cheminImage[12]=(i/10)+48;
		(*surface)[i] = SDL_LoadBMP(voiture->cheminImage);
		if ((*surface)[i] == NULL) {
			printf("Unable to load bitmap: %s\n", SDL_GetError());
			return 1;
		}
		SDL_SetColorKey((*surface)[i], SDL_SRCCOLORKEY, SDL_MapRGB((*surface)[i]->format, 97, 68, 43));
	}
	voiture->tabVoiture=malloc(nbImages*sizeof(int **));
	if(voiture->tabVoiture==NULL)return 4;
	for(n=0;n<nbImages;n++){
		voiture->tabVoiture[n]=malloc(largeur*sizeof(int *));
		if(voiture->tabVoiture[n]==NULL)return 4;
		for(i=0; i< largeur; i++) {
			voiture->tabVoiture[n][i] = calloc(hauteur,sizeof(int));
			if(voiture->tabVoiture[n][i]==NULL)return 4;
		}
		chargerMasque(voiture->tabVoiture[n], largeur,hauteur, (*surface)[n]);
	}
	return 0;
}


int initialisation (Camera *camera, Voiture voitures[], Circuit * circuit, int nbrDeJoueurs, Partie partie) {
	int i,j;
	char *** tab;	
	SDL_Surface *surfaceMasque;
	char chemin[50];
		
	//initialisation du circuit
	sscanf (partie.nomsCircuits[partie.circuit],"%s",circuit->nomCircuit);
	lireVariables(circuit);

	//initialisation des voitures 
	for (i=0; i<nbrDeJoueurs; i++) 
		initialisationVoitures (&voitures[i], partie, circuit, i+1);
	
	tab=(char ***)calloc(circuit->nbrImageY,sizeof(char**)); 
	if(tab==NULL)return 4;

	for(i=0;i<circuit->nbrImageY;i++)  {
		tab[i]=(char **) calloc( circuit->nbrImageX ,sizeof(char*));
		if(tab[i]==NULL)return 4;
		for(j=0;j<circuit->nbrImageX;j++) {
			tab[i][j]=(char *) calloc( 1024 ,sizeof(char));
			if(tab[i][j]==NULL)return 4;
			sprintf(chemin,"Circuit/%s%d%d.bmp",circuit->nomCircuit,i,j);
			strcpy(tab[i][j],chemin);
		}
	}
	circuit->image=tab;
	
	//initialisation camera
	for (i=0; i<4; i++)
		camera->fond[i] = NULL; 
	camera->positionVoitures = malloc(nbrDeJoueurs * sizeof(SDL_Rect));
	if(camera->positionVoitures==NULL)return 4;
	for (i=0; i<nbrDeJoueurs; i++) {
		if(i == 0) 
			camera->positionVoitures[i].y = 300;
		else
			camera->positionVoitures[i].y = 350;
		
		camera->positionVoitures[i].x = 400;
	}
	camera->coin[0] =  camera->coin[1] = 1;
	camera->coinprec[0] = camera->coinprec[1] = 0;
	camera->nbrTour = circuit->nbTours;
	camera->tourActuel = 0;
	camera->temps = 0;
	camera->points = 0;
	
	//allocation des voitures	
	camera->spriteVoiture = malloc(nbrDeJoueurs * sizeof(SDL_Surface **));
	if(camera->spriteVoiture==NULL)return 4;

	for (i=0; i<nbrDeJoueurs; i++) {
		camera->spriteVoiture[i] = malloc(32 * sizeof(SDL_Surface *));
		if(camera->spriteVoiture[i]==NULL)return 4;
		allocationVoiture(&(camera->spriteVoiture[i]), &voitures[i]);
	}

	//chargement du masque collisions
	sprintf(chemin,"Circuit/%s_masque.bmp",circuit->nomCircuit);
	surfaceMasque = SDL_LoadBMP(chemin);
	circuit->tabMasque=malloc(circuit->largeurImage*circuit->nbrImageX*sizeof(int *));
	if(circuit->tabMasque==NULL)return 4;
	for(i=0; i< circuit->largeurImage * circuit->nbrImageX; i++) {
		circuit->tabMasque[i] = calloc(circuit->hauteurImage*circuit->nbrImageY,sizeof(int));
		if(circuit->tabMasque[i]==NULL)return 4;
	}
	chargerMasque(circuit->tabMasque, circuit->largeurImage * circuit->nbrImageX, circuit->hauteurImage * circuit->nbrImageY, surfaceMasque);
	
	//chargement du masque checkpoints
	sprintf(chemin,"Circuit/%s_checkpoints.bmp",circuit->nomCircuit);
	surfaceMasque = SDL_LoadBMP(chemin);
	circuit->tabCheckpoints=malloc(circuit->largeurImage*circuit->nbrImageX*sizeof(int *));
	if(circuit->tabCheckpoints==NULL)return 4;
	for(i=0; i < circuit->largeurImage * circuit->nbrImageX; i++) {
		circuit->tabCheckpoints[i]=calloc(circuit->hauteurImage*circuit->nbrImageY,sizeof(int));
		if(circuit->tabCheckpoints[i]==NULL)return 4;
	}
	chargerMasque(circuit->tabCheckpoints,circuit->largeurImage*circuit->nbrImageX,circuit->hauteurImage*circuit->nbrImageY, surfaceMasque);
	
	SDL_FreeSurface(surfaceMasque);
	return 0;
}


int liberation(Voiture *voiture, Circuit *circuit, Camera * camera, int nbrDeJoueurs) {
	int i,j,k;
	int largeur=96;

	//liberation des voitures 
	
	for(i=0;i<circuit->nbrImageY;i++)  {
		for(j=0;j<circuit->nbrImageX;j++) {
			free(circuit->image[i][j]);
		}
		free(circuit->image[i]);
	}
	free(circuit->image);
	
	
	//liberation camera
	for (i=0; i<4; i++)
		SDL_FreeSurface(camera->fond[i]); 
	
	free(camera->positionVoitures) ;
	
	for (i=0; i<nbrDeJoueurs; i++) {
		
		for (j=0;j<32;j++) {
			SDL_FreeSurface(camera->spriteVoiture[i][j]);
		}
		free(camera->spriteVoiture[i]);
		
		for (j=0;j<32;j++) {
			for(k=0;k<largeur;k++)
				free(voiture[i].tabVoiture[j][k]);
			free(voiture[i].tabVoiture[j]);
		}
		free(voiture[i].tabVoiture);
			
	}
	
	free (camera->spriteVoiture);
	
	//liberation du masque collisions
	for(i=0; i< circuit->largeurImage * circuit->nbrImageX; i++) {
		free(circuit->tabMasque[i]);
	}
	free(circuit->tabMasque);
	
	//liberation du masque checkpoints
	for(i=0; i< circuit->largeurImage * circuit->nbrImageX; i++) {
		free(circuit->tabCheckpoints[i]);
	}
	free(circuit->tabCheckpoints);
	

	
	return 0;
}

void repositionnerVoitures (int voitureEnTete, Voiture * voitures) {
	voitures[0].position = voitures[voitureEnTete].position;
	voitures[1].position = voitures[voitureEnTete].position;
	voitures[0].checkpoints = voitures[voitureEnTete].checkpoints;
	voitures[1].checkpoints = voitures[voitureEnTete].checkpoints;
	voitures[0].couleurPrec = voitures[voitureEnTete].couleurPrec;
	voitures[1].couleurPrec = voitures[voitureEnTete].couleurPrec;
	voitures[0].couleurPrecPrec = voitures[voitureEnTete].couleurPrecPrec;
	voitures[1].couleurPrecPrec = voitures[voitureEnTete].couleurPrecPrec;
	voitures[0].angle = voitures[voitureEnTete].angle;
	voitures[1].angle = voitures[voitureEnTete].angle;

}

void gestion2j (Voiture * voitures, Camera * camera, int *compt, Partie * partie, int *done) {
	if(abs(voitures[0].position.y - voitures[1].position.y)>=560 || abs(voitures[0].position.x - voitures[1].position.x)>=760) {
		if(voitures[0].checkpoints < voitures[1].checkpoints) {
			camera->points++;
			repositionnerVoitures(1, voitures);
		}
		else {
			camera->points--;
			repositionnerVoitures(0, voitures);
		}
		*compt = 2;
	}
	if(abs(camera->points) == 3) {
		partie->menu = MenuFinA;
		*done = 1;
	}
}

void gestion1j (Voiture *voitures, Camera * camera, Circuit circuit, Partie *partie, int *done, Scores *scores) {
	//Gestion des tours
	if(voitures[0].checkpoints == circuit.totalCheckpoints) {
		camera->tourActuel++;
		voitures[0].checkpoints = 1;
	}
	
	//Gestion de la fin de la course
	if(camera->tourActuel == camera->nbrTour) {
		*done = 1;	//Arret de gestion circuit
		partie->timer=camera->temps;
		lireScores(partie, scores);
		if (scores->temps[4] < partie->timer) 	//si le temps est superieur au dernier temps enregistre : game over
			partie->menu =MenuFinB;
		else{
			insererScore(partie, scores);
			partie->menu =MenuFinA;
		}
	}
}


void gestionPause(SDL_Event *event, Partie *partie, SDL_Surface * ecran, int *done, int *tempsPause, Scores *scores) {
	int boucle, tempsAvantPause;
	
	//Sauvegarde du temps avant la pause
	tempsAvantPause = SDL_GetTicks();

	//vidage du cache des evenements
	boucle = 1;
	while (boucle) {
		SDL_PollEvent(event);
		if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_ESCAPE)
			boucle= 0;
	}
	
	//mise en pause
	partie->menu = MenuPause;				
	gestionMenu (ecran, partie, scores);
	if(partie->menu != MenuJeu) {
		*done =1; //Arret de gestion circuit
	}
	
	//Calcul du temps de pause
	*tempsPause += SDL_GetTicks() - tempsAvantPause;	
}


void gestionEvent(SDL_Event event, Voiture * voitures, Partie *partie, int nbrDeJoueurs, int *done) {
	if(event.type==SDL_KEYDOWN){
		if((event.key.keysym.sym)==partie->clavier.hJoueur1) voitures[0].haut=1;
		if((event.key.keysym.sym)==partie->clavier.bJoueur1) voitures[0].bas=1;
		if((event.key.keysym.sym)==partie->clavier.gJoueur1) voitures[0].gauche=1;
		if((event.key.keysym.sym)==partie->clavier.dJoueur1) voitures[0].droite=1;
		if(nbrDeJoueurs == 2) {
			if((event.key.keysym.sym)==partie->clavier.hJoueur2) voitures[1].haut=1;
			if((event.key.keysym.sym)==partie->clavier.bJoueur2) voitures[1].bas=1;
			if((event.key.keysym.sym)==partie->clavier.gJoueur2) voitures[1].gauche=1;
			if((event.key.keysym.sym)==partie->clavier.dJoueur2) voitures[1].droite=1;
		}
		if((event.key.keysym.sym)==SDLK_ESCAPE) partie->pause = 1;
	}
	if(event.type==SDL_KEYUP){
		if((event.key.keysym.sym)==partie->clavier.hJoueur1) voitures[0].haut=0;
		if((event.key.keysym.sym)==partie->clavier.bJoueur1) voitures[0].bas=0;
		if((event.key.keysym.sym)==partie->clavier.gJoueur1) voitures[0].gauche=0;
		if((event.key.keysym.sym)==partie->clavier.dJoueur1) voitures[0].droite=0;
		if((event.key.keysym.sym)==SDLK_ESCAPE) partie->pause = 0;
		if(nbrDeJoueurs == 2) {
			if((event.key.keysym.sym)==partie->clavier.hJoueur2) voitures[1].haut=0;
			if((event.key.keysym.sym)==partie->clavier.bJoueur2) voitures[1].bas=0;
			if((event.key.keysym.sym)==partie->clavier.gJoueur2) voitures[1].gauche=0;
			if((event.key.keysym.sym)==partie->clavier.dJoueur2) voitures[1].droite=0;
		}
	}
	if(event.type==SDL_QUIT) {
		partie->menu = MenuQuitter;
		*done = 1; //Arret de gestion circuit
	}		
	
}

int gestionCircuit( SDL_Surface *ecran, Partie *partie, Scores *scores) {
	
	int done,i, compt;
	int tempsPrecedent = 0, tempsActuel = 0, tempsDebutCourse, tempsPause = 0;
	int nbrDeJoueurs;

	SDL_Event event;

	Voiture * voitures;
	Circuit circuit;
	Camera camera;
	
	//Allocation du tableau des voitures
	nbrDeJoueurs = partie->nbrDeJoueur;
	voitures = malloc(nbrDeJoueurs * sizeof(Voiture));
	if(voitures==NULL)return 4;

	//Chargement des parametress de la course
	ecranChargement (ecran);
	if(initialisation(&camera, voitures, &circuit, nbrDeJoueurs, *partie)==1){
		return 3;
	}
	
	//Initilaisation de la boucle principale
	tempsDebutCourse = SDL_GetTicks();
	compt = 3;
	SDL_EnableKeyRepeat(10, 10);
	done = 0;
	
	while ( !done ) {
		tempsActuel = SDL_GetTicks();
		if (tempsActuel - tempsPrecedent > 30){ //Traitement d'une image toutes les 30ms
			tempsPrecedent = tempsActuel;
			if(compt != -1) {
				//Affichage du decompte
				affichage(ecran,voitures,circuit,&camera,nbrDeJoueurs);
				affichageDecompte(ecran, &camera, &compt, &tempsDebutCourse,tempsPause);
				SDL_Flip(ecran);
			}
			else {
				//Deplacement des voitures
				for(i=0; i<nbrDeJoueurs; i ++)
					deplacer(&voitures[i],circuit,camera.spriteVoiture[i]);
				
				//Gestion temps et joueurs
				camera.temps = tempsActuel - tempsDebutCourse - tempsPause;
				if(nbrDeJoueurs == 1)
					gestion1j (voitures, &camera, circuit, partie, &done, scores);
				else
					gestion2j(voitures, &camera, &compt, partie, &done);
				
				//Affichage de l'ecran calcule
				affichage(ecran,voitures,circuit,&camera,nbrDeJoueurs);
				SDL_Flip(ecran);
			}
			
			//Gestion  de la pause
			if(partie->pause == 1)
				gestionPause(&event, partie, ecran, &done, &tempsPause, scores);
			
			//Gestion des evenements
			SDL_PollEvent(&event);
			gestionEvent(event, voitures, partie, nbrDeJoueurs, &done);
		}
		else // Si ça fait moins de 30ms depuis le dernier tour de boucle, on endort le programme le temps qu'il faut
		{
			SDL_Delay(30 - (tempsActuel - tempsPrecedent));
		}
	}
	
	liberation(voitures, &circuit, &camera, nbrDeJoueurs);	
	free(voitures);
	
	return 0;
}