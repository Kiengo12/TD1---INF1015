/**
* Programme qui charge une collection de groupes musicaux d'un fichier binaire
* en allocation dynamique, l'affiche, la modifie puis la détruit sans fuite.
* \file   td2_partie_02.cpp
* \author Cedric KIENGO && Yorrick CABO
* \date   27 septembre 2026
*/

#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS

#include "td2_partie_02.hpp"

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


static span<Musicien*> obtenirMembres(const Groupe& groupe)
{
	return span<Musicien*>(groupe.membres.elements,
		static_cast<size_t>(groupe.membres.nElements));
}


#pragma region "ListeGroupes"//{
ListeGroupes::ListeGroupes()
	: capacite_(0),
	nElements_(0),
	elements_(nullptr)
{
}


ListeGroupes::ListeGroupes(int capaciteInitiale)
	: capacite_(capaciteInitiale),
	nElements_(0),
	elements_(new Groupe* [capaciteInitiale])
{
}


int ListeGroupes::obtenirNElements() const
{
	return nElements_;
}


span<Groupe* const> ListeGroupes::obtenirGroupes() const
{
	return span<Groupe* const>(elements_, static_cast<size_t>(nElements_));
}


void ListeGroupes::ajouterGroupe(Groupe* groupe)
{
	if (nElements_ == capacite_) {
		int nouvelleCapacite = max(1, capacite_ * facteurCroissance);
		Groupe** nouveauxElements = new Groupe * [nouvelleCapacite];
		for (int i : range(nElements_))
			nouveauxElements[i] = elements_[i];
		delete[] elements_;
		elements_ = nouveauxElements;
		capacite_ = nouvelleCapacite;
	}
	elements_[nElements_] = groupe;
	nElements_++;
}


void ListeGroupes::retirerGroupe(Groupe* groupe)
{
	for (int i : range(nElements_)) {
		if (elements_[i] == groupe) {
			nElements_--;
			elements_[i] = elements_[nElements_];
			return;
		}
	}
}


Musicien* ListeGroupes::trouverMusicien(const string& nom) const
{
	for (Groupe* groupe : obtenirGroupes()) {
		for (Musicien* musicien : obtenirMembres(*groupe)) {
			if (musicien->nom == nom)
				return musicien;
		}
	}
	return nullptr;
}


void ListeGroupes::liberer()
{
	delete[] elements_;
	elements_ = nullptr;
	capacite_ = 0;
	nElements_ = 0;
}
#pragma endregion//}

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

#pragma region "Création et destruction de la collection"//{
Musicien* lireMusicien(istream& fichier, const ListeGroupes& listeGroupes)
{
	string nom = lireString(fichier);
	string pays = lireString(fichier);
	int    anneeNaissance = lireEntier(fichier);

	Musicien* musicien = listeGroupes.trouverMusicien(nom);
	if (musicien == nullptr) {
		musicien = new Musicien{ nom, pays, anneeNaissance,
								  ListeGroupes() };
		cout << musicien->nom << endl;
	}
	return musicien;
}


Groupe* lireGroupe(istream& fichier, const ListeGroupes& listeGroupes)
{
	string nom = lireString(fichier);
	string genre = lireString(fichier);
	int    anneeFormation = lireEntier(fichier);
	int    nMembres = lireEntier(fichier);

	Musicien** membres = new Musicien * [nMembres];
	Groupe* groupe = new Groupe{ nom, genre, anneeFormation,
									 { nMembres, nMembres, membres } };
	cout << groupe->nom << endl;

	for (int i : range(nMembres)) {
		Musicien* musicien = lireMusicien(fichier, listeGroupes);
		musicien->joueDans.ajouterGroupe(groupe);
		membres[i] = musicien;
	}
	return groupe;
}


ListeGroupes creerListe(const string& nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nGroupes = lireEntier(fichier);
	ListeGroupes listeGroupes(nGroupes);
	for ([[maybe_unused]] int i : range(nGroupes))
		listeGroupes.ajouterGroupe(lireGroupe(fichier, listeGroupes));
	return listeGroupes;
}


void detruireGroupe(Groupe* groupe)
{
	for (Musicien* musicien : obtenirMembres(*groupe)) {
		musicien->joueDans.retirerGroupe(groupe);
		if (musicien->joueDans.obtenirNElements() == 0) {
			cout << musicien->nom << endl;
			musicien->joueDans.liberer();
			delete musicien;
		}
	}
	delete[] groupe->membres.elements;
	delete groupe;
}


void detruireListe(ListeGroupes& listeGroupes)
{
	for (Groupe* groupe : listeGroupes.obtenirGroupes())
		detruireGroupe(groupe);
	listeGroupes.liberer();
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
	for (Groupe* groupe : listeGroupes.obtenirGroupes()) {
		afficherGroupe(*groupe);
		cout << ligneDeSeparation;
	}
}


void afficherGroupesMusicien(const ListeGroupes& listeGroupes,
	const string& nomMusicien)
{
	const Musicien* musicien = listeGroupes.trouverMusicien(nomMusicien);
	if (musicien == nullptr)
		cout << "Aucun musicien de ce nom" << endl;
	else
		afficherListeGroupes(musicien->joueDans);
}
#pragma endregion//}

int main()
{
	bibliotheque_cours::activerCouleursAnsi();

	static const string ligneDeSeparation =
		"\n\033[35m════════════════════════════════════════\033[0m\n";

	ListeGroupes listeGroupes = creerListe("groupes.bin");

	cout << ligneDeSeparation << "Le premier groupe de la liste est:" << endl;
	afficherGroupe(*listeGroupes.obtenirGroupes()[0]);

	cout << ligneDeSeparation << "Les groupes sont:" << endl;
	afficherListeGroupes(listeGroupes);

	Musicien* daveGrohl = listeGroupes.trouverMusicien(nomDaveGrohl);
	assert(daveGrohl != nullptr);
	daveGrohl->anneeNaissance = anneeNaissanceDaveGrohl;

	cout << ligneDeSeparation << "Liste des groupes où Dave Grohl joue sont:"
		<< endl;
	afficherGroupesMusicien(listeGroupes, nomDaveGrohl);

	Groupe* premierGroupe = listeGroupes.obtenirGroupes()[0];
	listeGroupes.retirerGroupe(premierGroupe);
	detruireGroupe(premierGroupe);

	cout << ligneDeSeparation << "Les groupes sont maintenant:" << endl;
	afficherListeGroupes(listeGroupes);

	// Couvre le cas où le musicien cherché n'existe pas.
	afficherGroupesMusicien(listeGroupes, "Musicien inexistant");

	detruireListe(listeGroupes);
}