/**
* Programme qui charge une collection de groupes musicaux d'un fichier binaire
* en allocation dynamique, l'affiche, la modifie puis la détruit sans fuite.
* \file   td2.cpp
* \author Cedric KIENGO && Yorrick CABO
* \date   27 septembre 2026
*/

#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS

#include "structures.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <span>
#include <string>

#include "cppitertools/range.hpp"

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp"

#include "debogage_memoire.hpp"

using namespace std;
using namespace iter;

#pragma endregion//}

static const int    facteurCroissance = 2;
static const int    anneeNaissanceDaveGrohl = 1969;
static const string nomDaveGrohl = "Dave Grohl";

#pragma region "Fonctions de base pour lire le fichier binaire"//{
template <typename T>
T lireType(istream& fichier)
{
	T valeur{};
	fichier.read(reinterpret_cast<char*>(&valeur), sizeof(valeur));
	return valeur;
}
#define erreurFataleAssert(message) assert(false && (message)), terminate()
static const uint8_t enteteTailleVariableDeBase = 0xA0;
size_t lireUintTailleVariable(istream& fichier)
{
	uint8_t entete = lireType<uint8_t>(fichier);
	switch (entete) {
	case enteteTailleVariableDeBase + 0:
		return lireType<uint8_t>(fichier);
	case enteteTailleVariableDeBase + 1:
		return lireType<uint16_t>(fichier);
	case enteteTailleVariableDeBase + 2:
		return lireType<uint32_t>(fichier);
	default:
		cerr << "À la position de fichier "
			<< streamoff(fichier.tellg()) - 1 << ": ";
		erreurFataleAssert("Tentative de lire un entier de taille variable"
			" alors que le fichier contient autre chose à"
			" cet emplacement.");
	}
}


int lireEntier(istream& fichier)
{
	return static_cast<int>(lireUintTailleVariable(fichier));
}


string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUintTailleVariable(fichier));
	fichier.read(texte.data(),
		streamsize(sizeof(texte[0])) * streamsize(texte.length()));
	return texte;
}

#pragma endregion//}

#pragma region "Gestion des listes"//{
span<Groupe*> obtenirGroupes(const ListeGroupes& listeGroupes)
{
	return span<Groupe*>(listeGroupes.elements,
		static_cast<size_t>(listeGroupes.nElements));
}


span<Musicien*> obtenirMembres(const Groupe& groupe)
{
	return span<Musicien*>(groupe.membres.elements,
		static_cast<size_t>(groupe.membres.nElements));
}


void ajouterGroupe(ListeGroupes& listeGroupes, Groupe* groupe)
{
	if (listeGroupes.nElements == listeGroupes.capacite) {
		int nouvelleCapacite = max(1,
			listeGroupes.capacite * facteurCroissance);
		Groupe** nouveauxElements = new Groupe * [nouvelleCapacite];

		for (int i : range(listeGroupes.nElements))
			nouveauxElements[i] = listeGroupes.elements[i];

		delete[] listeGroupes.elements;

		listeGroupes.elements = nouveauxElements;
		listeGroupes.capacite = nouvelleCapacite;
	}

	listeGroupes.elements[listeGroupes.nElements] = groupe;
	listeGroupes.nElements++;
}


void retirerGroupe(ListeGroupes& listeGroupes, Groupe* groupe)
{
	for (int i : range(listeGroupes.nElements)) {
		if (listeGroupes.elements[i] == groupe) {
			int dernier = listeGroupes.nElements - 1;
			listeGroupes.elements[i] = listeGroupes.elements[dernier];
			listeGroupes.elements[dernier] = nullptr;
			listeGroupes.nElements--;
			return;
		}
	}
}


Musicien* trouverMusicien(const ListeGroupes& listeGroupes, const string& nom)
{
	for (Groupe* groupe : obtenirGroupes(listeGroupes)) {
		for (Musicien* musicien : obtenirMembres(*groupe)) {
			if (musicien->nom == nom)
				return musicien;
		}
	}
	return nullptr;
}

#pragma endregion//}

#pragma region "Création et destruction de la collection"//{
Musicien* lireMusicien(istream& fichier, const ListeGroupes& listeGroupes)
{
	string nom = lireString(fichier);
	string pays = lireString(fichier);
	int    anneeNaissance = lireEntier(fichier);

	Musicien* musicienExistant = trouverMusicien(listeGroupes, nom);
	if (musicienExistant != nullptr)
		return musicienExistant;

	Musicien* nouveauMusicien = new Musicien{ nom, pays, anneeNaissance,
											  { 0, 0, nullptr } };
	cout << nouveauMusicien->nom << endl;
	return nouveauMusicien;
}


Groupe* lireGroupe(istream& fichier, const ListeGroupes& listeGroupes)
{
	Groupe* groupe = new Groupe{};
	groupe->nom = lireString(fichier);
	groupe->genre = lireString(fichier);
	groupe->anneeFormation = lireEntier(fichier);
	groupe->membres.nElements = lireEntier(fichier);
	groupe->membres.capacite = groupe->membres.nElements;
	groupe->membres.elements = new Musicien * [groupe->membres.capacite];

	cout << groupe->nom << endl;

	for (int i : range(groupe->membres.nElements)) {
		Musicien* musicien = lireMusicien(fichier, listeGroupes);
		groupe->membres.elements[i] = musicien;
		ajouterGroupe(musicien->joueDans, groupe);
	}
	return groupe;
}


ListeGroupes creerListe(const string& nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nGroupes = lireEntier(fichier);

	ListeGroupes listeGroupes = { nGroupes, 0, new Groupe * [nGroupes] };
	for ([[maybe_unused]] int i : range(nGroupes)) {
		Groupe* groupe = lireGroupe(fichier, listeGroupes);
		ajouterGroupe(listeGroupes, groupe);
	}
	return listeGroupes;
}


void detruireGroupe(Groupe* groupe)
{
	for (Musicien* musicien : obtenirMembres(*groupe)) {
		retirerGroupe(musicien->joueDans, groupe);

		if (musicien->joueDans.nElements == 0) {
			cout << musicien->nom << endl;
			delete[] musicien->joueDans.elements;
			delete musicien;
		}
	}

	delete[] groupe->membres.elements;
	delete groupe;
}


void detruireListe(ListeGroupes& listeGroupes)
{
	for (Groupe* groupe : obtenirGroupes(listeGroupes))
		detruireGroupe(groupe);

	delete[] listeGroupes.elements;

	listeGroupes.elements = nullptr;
	listeGroupes.nElements = 0;
	listeGroupes.capacite = 0;
}

#pragma endregion//}

#pragma region "Affichage"//{
void afficherMusicien(const Musicien& musicien)
{
	cout << "  " << musicien.nom << ", " << musicien.pays << ", "
		<< musicien.anneeNaissance << endl;
}


void afficherGroupe(const Groupe& groupe)
{
	cout << "Nom: " << groupe.nom << endl;
	cout << "Genre: " << groupe.genre << endl;
	cout << "Année de formation: " << groupe.anneeFormation << endl;
	cout << "Membres:" << endl;

	for (Musicien* musicien : obtenirMembres(groupe))
		afficherMusicien(*musicien);
}


void afficherListeGroupes(const ListeGroupes& listeGroupes)
{
	static const string ligneDeSeparation =
		"\n────────────────────────────────────────\n";

	cout << ligneDeSeparation;
	for (Groupe* groupe : obtenirGroupes(listeGroupes)) {
		afficherGroupe(*groupe);
		cout << ligneDeSeparation;
	}
}


void afficherGroupesMusicien(const ListeGroupes& listeGroupes,
	const string& nomMusicien)
{
	const Musicien* musicien = trouverMusicien(listeGroupes, nomMusicien);
	if (musicien == nullptr)
		cout << "Aucun musicien de ce nom" << endl;
	else
		afficherListeGroupes(musicien->joueDans);
}

#pragma endregion//}

int main()
{
	// Active les codes de couleurs ANSI dans la console Windows.
	bibliotheque_cours::activerCouleursAnsi();

	static const string ligneDeSeparation =
		"\n\033[35m════════════════════════════════════════\033[0m\n";

	ListeGroupes listeGroupes = creerListe("groupes.bin");

	cout << ligneDeSeparation << "Le premier groupe de la liste est:" << endl;
	afficherGroupe(*listeGroupes.elements[0]);

	cout << ligneDeSeparation << "Les groupes sont:" << endl;
	afficherListeGroupes(listeGroupes);

	Musicien* daveGrohl = trouverMusicien(listeGroupes, nomDaveGrohl);
	assert(daveGrohl != nullptr);
	daveGrohl->anneeNaissance = anneeNaissanceDaveGrohl;

	cout << ligneDeSeparation << "Liste des groupes où Dave Grohl joue sont:"
		<< endl;
	afficherGroupesMusicien(listeGroupes, nomDaveGrohl);

	Groupe* groupeADetruire = listeGroupes.elements[0];
	retirerGroupe(listeGroupes, groupeADetruire);
	detruireGroupe(groupeADetruire);

	cout << ligneDeSeparation << "Les groupes sont maintenant:" << endl;
	afficherListeGroupes(listeGroupes);

	// Couvre le cas où le musicien cherche n'existe pas.
	afficherGroupesMusicien(listeGroupes, "Musicien inexistant");

	detruireListe(listeGroupes);
}