#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#ifdef __APPLE__
#include <SDL_ttf/SDL_ttf.h>
#else
#include <SDL/SDL_ttf.h>
#endif
#include "gestionCircuit.h"
#include "affichage.h"
#include "camera.h"
#include "menu.h"


void affichage(SDL_Surface *ecran, Voiture voiture, Circuit circuit, Camera *camera){
	
	char phrase[10];
	
	SDL_Color couleurBlanc = {255, 255, 255};
	SDL_Rect positionTexte;
	SDL_Surface *texte;
	TTF_Font *police = TTF_OpenFont("Prototype.ttf", 20);

	
	//Affichage voiture et fond
	voiture.image=camera->spriteVoiture[0][(voiture.angle)];  
        positionnerCamera (ecran, circuit, voiture, camera);
        SDL_BlitSurface(voiture.image, NULL, ecran, &(camera->positionVoitures[0]));
       // SDL_Flip(ecran);
	
	//Affichage du temps
	
	positionTexte.x = 30;
	positionTexte.y = 30;
	
	sprintf(phrase, "%d:%02d:%02d",camera->temps/60000,camera->temps/1000 % 60, camera->temps/10 % 100);
	texte = TTF_RenderText_Blended(police, phrase, couleurBlanc);
	SDL_BlitSurface(texte, NULL, ecran, &positionTexte);
	
	SDL_Flip(ecran);
	
	TTF_CloseFont(police); // Fermeture de la police 

}



int ecranChargement (SDL_Surface * ecran) {
	SDL_Color couleurBlanc = {255, 255, 255};
	SDL_Rect position;
	char phrase[] ="Chargement";
	SDL_Surface * fond, * texte;
	TTF_Font *police = TTF_OpenFont("Prototype.ttf", 50);
	
	//Positionnement du fond
	position.x = 0;
	position.y = 0;
	
	//Création et affichage d'un fond noir
	fond = SDL_CreateRGBSurface(SDL_HWSURFACE, 800, 600, 32, 0, 0, 0, 0);
	SDL_FillRect(fond, NULL, SDL_MapRGB(fond->format, 0, 0, 0));
	SDL_BlitSurface(fond, NULL, ecran, &position);
	
	
	
	//Création du texte
	texte = TTF_RenderText_Blended(police, phrase, couleurBlanc);
	
	//Positionnement du texte
	position.x = 400 - texte->w /2;
	position.y = 300 - texte->h /2;
	
	SDL_BlitSurface(texte, NULL, ecran, &position);
	
	SDL_Flip(ecran);
	
	TTF_CloseFont(police);
	SDL_FreeSurface(texte);
	SDL_FreeSurface(fond);
	
	return 0;
}

int affichageDecompte(SDL_Surface *ecran, Voiture voiture, Circuit circuit, Camera *camera) {
	
	int done,i;
	int tempsPrecedent = 0, tempsActuel = 0;
	char phrase[10];
	
	SDL_Color couleurBlanc = {255, 255, 255};
	SDL_Rect positionTexte;
	SDL_Surface *texte;
	TTF_Font *police = TTF_OpenFont("Prototype.ttf", 50);
	
	positionTexte.x = 400;
	positionTexte.y = 200;
	
	tempsActuel = SDL_GetTicks();
	tempsPrecedent = tempsActuel - 2001;
	done = 0;
	i = 3;
	
	while ( !done) {
		tempsActuel = SDL_GetTicks();
		if(tempsActuel - tempsPrecedent > 1000) {	//Attente d'1 seconde entre chaque affichage
			if(i == -1) {	//Si le compteur est fini on arrete la boucle
				done = 1;
			}
			else {
				//Affichage de la voiture et du fond
				voiture.image = camera->spriteVoiture[0][(voiture.angle)];  
				positionnerCamera (ecran, circuit, voiture, camera);
				SDL_BlitSurface(voiture.image, NULL, ecran, &(camera->positionVoitures[0]));
				
				//Affichage du decompte
				sprintf(phrase, "%d",i);
				texte = TTF_RenderText_Blended(police, phrase, couleurBlanc);
				SDL_BlitSurface(texte, NULL, ecran, &positionTexte);
				SDL_Flip(ecran);
				
				//decrementation du compteur
				i--;
				tempsPrecedent = tempsActuel;
			}
		}
		else {
			SDL_Delay(1000 - (tempsActuel - tempsPrecedent));
		}
	}
	
	TTF_CloseFont(police);
	SDL_FreeSurface(texte);
	
	return 0;
}
