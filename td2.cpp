/*******************************************************************************
 * INF1015 - Programmation orientee objet avancee
 * TD 2
 * Auteurs: Kiengo Cedric - CABO Yorrick
 ******************************************************************************/


#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures.hpp"      // Structures de données pour la collection de groupes musicaux en mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include <span>

#include "cppitertools/range.hpp"

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).

using namespace std;
using namespace iter;

#pragma endregion//}

#pragma region "Fonctions de base pour lire le fichier binaire"//{
template <typename T>
T lireType(istream& fichier)
{
	T valeur{};
	fichier.read(reinterpret_cast<char*>(&valeur), sizeof(valeur));
	return valeur;
}
#define erreurFataleAssert(message) assert(false&&(message)),terminate()
static const uint8_t enteteTailleVariableDeBase = 0xA0;
size_t lireUintTailleVariable(istream& fichier)
{
	uint8_t entete = lireType<uint8_t>(fichier);
	switch (entete) {
	case enteteTailleVariableDeBase + 0: return lireType<uint8_t>(fichier);
	case enteteTailleVariableDeBase + 1: return lireType<uint16_t>(fichier);
	case enteteTailleVariableDeBase + 2: return lireType<uint32_t>(fichier);
	default:
		cerr << "À la position de fichier " << streamoff(fichier.tellg()) - 1 << ": "; erreurFataleAssert("Tentative de lire un entier de taille variable alors que le fichier contient autre chose à cet emplacement.");
	}
}

string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUintTailleVariable(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion//}

//TODO: Une fonction pour ajouter un Groupe à une ListeGroupes, le groupe existant déjà; on veut uniquement ajouter le pointeur vers le groupe existant.  Cette fonction doit doubler la taille du tableau alloué, avec au minimum un élément, dans le cas où la capacité est insuffisante pour ajouter l'élément.  Il faut alors allouer un nouveau tableau plus grand, copier ce qu'il y avait dans l'ancien, et éliminer l'ancien trop petit.  Cette fonction ne doit copier aucun Groupe ni Musicien, elle doit copier uniquement des pointeurs.
void ListeGroupes::ajouterGroupe(Groupe* groupe)
{
	if (nElements_ == capacite_) {
		int nouvelleCapacite;

		if (capacite_ == 0)
			nouvelleCapacite = 1;
		else
			nouvelleCapacite = capacite_ * 2;

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

//TODO: Une fonction pour enlever un Groupe d'une ListeGroupes (enlever le pointeur) sans effacer le groupe; la fonction prenant en paramètre un pointeur vers le groupe à enlever.  L'ordre des groupes dans la liste n'a pas à être conservé.
void ListeGroupes::enleverGroupe(Groupe* groupe)
{
	for (int i = 0; i < nElements_; i++) {
		if (elements_[i] == groupe) {
			elements_[i] = elements_[nElements_ - 1];
			elements_[nElements_ - 1] = nullptr;
			nElements_--;

			return;
		}
	}
}

//TODO: Une fonction pour trouver un Musicien par son nom dans une ListeGroupes, qui retourne un pointeur vers le musicien, ou nullptr si le musicien n'est pas trouvé.  Devrait utiliser span.
Musicien* ListeGroupes::trouverMusicien(const string& nom) const
{
	span<Groupe*> groupes(elements_, nElements_);

	for (Groupe* groupe : groupes) {
		span<Musicien*> musiciens(
			groupe->membres.elements,
			groupe->membres.nElements
		);

		for (Musicien* musicien : musiciens) {
			if (musicien->nom == nom)
				return musicien;
		}
	}

	return nullptr;
}

//TODO: Compléter les fonctions pour lire le fichier et créer/allouer une ListeGroupes.  La ListeGroupes devra être passée entre les fonctions, pour vérifier l'existence d'un Musicien avant de l'allouer à nouveau (cherché par nom en utilisant la fonction ci-dessus).
Musicien* lireMusicien(istream& fichier, const ListeGroupes& listeGroupes)
{
	Musicien musicien = {};
	musicien.nom = lireString(fichier);
	musicien.pays = lireString(fichier);
	musicien.anneeNaissance = lireUintTailleVariable(fichier);

	Musicien* musicienExistant =
		listeGroupes.trouverMusicien(musicien.nom);

	if (musicienExistant != nullptr) {
		return musicienExistant;//TODO: Retourner un pointeur soit vers un musicien existant ou un nouveau musicien ayant les bonnes informations, selon si le musicien existait déjà.  Pour fins de débogage, affichez les noms des membres crées; vous ne devriez pas voir le même nom de musicien affiché deux fois pour la création.
	}

	Musicien* nouveauMusicien = new Musicien;
	*nouveauMusicien = musicien;

	cout << nouveauMusicien->nom << endl;

	return nouveauMusicien;
}

Groupe* lireGroupe(istream& fichier, ListeGroupes& listeGroupes)
{
	Groupe* groupe = new Groupe{};
	groupe->nom = lireString(fichier);
	groupe->genre = lireString(fichier);
	groupe->anneeFormation = lireUintTailleVariable(fichier);
	groupe->membres.nElements = lireUintTailleVariable(fichier);  //NOTE: Vous avez le droit d'allouer d'un coup le tableau pour les membres, sans faire de réallocation comme pour ListeGroupes.  Vous pouvez aussi copier-coller les fonctions d'allocation de ListeGroupes ci-dessus dans des nouvelles fonctions et faire un remplacement de Groupe par Musicien, pour réutiliser cette réallocation.

	groupe->membres.capacite = groupe->membres.nElements;
	groupe->membres.elements = new Musicien * [groupe->membres.capacite];

	cout << groupe->nom << endl;

	for (int i = 0; i < groupe->membres.nElements; i++) {
		groupe->membres.elements[i] = lireMusicien(fichier, listeGroupes); //TODO: Placer le musicien au bon endroit dans les membres du groupe.
		//TODO: Ajouter le groupe à la liste des groupes dans lesquels le musicien joue.
		Musicien* musicien = groupe->membres.elements[i];

		musicien->joueDans.ajouterGroupe(groupe);
	}
	return groupe; //TODO: Retourner le pointeur vers le nouveau groupe.
}

ListeGroupes::ListeGroupes(const string& nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nombreGroupes = lireUintTailleVariable(fichier);

	//TODO: Créer une liste de groupes vide.
	for (int i = 0; i < nombreGroupes; i++) {
		Groupe* groupe = lireGroupe(fichier, *this); //TODO: Ajouter le groupe à la liste.
		ajouterGroupe(groupe);
	}
	//TODO: Retourner la liste de groupes.
}

//TODO: Une fonction pour détruire un groupe (relâcher toute la mémoire associée à ce groupe, et les membres qui ne jouent plus dans aucun groupes de la collection).  Noter qu'il faut enleve le groupe détruit des groupes dans lesquels jouent les membres.  Pour fins de débogage, affichez les noms des membres lors de leur destruction.
void detruireGroupe(Groupe* groupe)
{
	for (int i : range(groupe->membres.nElements)) {
		Musicien* musicien = groupe->membres.elements[i];

		musicien->joueDans.enleverGroupe(groupe);
		if (musicien->joueDans.size() == 0) {
			cout << musicien->nom << endl;

			musicien->joueDans.detruire();
			delete musicien;
		}
	}

	delete[] groupe->membres.elements;
	delete groupe;
}

//TODO: Une fonction pour détruire une ListeGroupes et tous les groupes qu'elle contient.
void ListeGroupes::detruire()
{
	for (int i : range(nElements_)) {
		detruireGroupe(elements_[i]);
	}

	delete[] elements_;

	elements_ = nullptr;
	nElements_ = 0;
	capacite_ = 0;
}

void afficherMusicien(const Musicien& musicien)
{
	cout << "  " << musicien.nom << ", " << musicien.pays << ", " << musicien.anneeNaissance << endl;
}

//TODO: Une fonction pour afficher un groupe avec tous ses membres (en utilisant la fonction afficherMusicien ci-dessus).
void afficherGroupe(const Groupe& groupe)
{
	cout << "Nom: " << groupe.nom << endl;
	cout << "Genre: " << groupe.genre << endl;
	cout << "Annee de formation: " << groupe.anneeFormation << endl;
	cout << "Membres:" << endl;

	for (int i : range(groupe.membres.nElements)) {
		afficherMusicien(*groupe.membres.elements[i]);
	}
}


void ListeGroupes::afficher() const
{
	//TODO: Utiliser des caractères Unicode pour définir la ligne de séparation (différente des autres lignes de séparations dans ce progamme).
	static const string ligneDeSeparation =
		"\n════════════════════════════════════════\n";

	cout << ligneDeSeparation;

	span<Groupe*> groupes(elements_, nElements_);

	//TODO: Changer le for pour utiliser un span.
	for (Groupe* groupe : groupes) {
		afficherGroupe(*groupe); //TODO: Afficher le groupe.
		cout << ligneDeSeparation;
	}
}

void ListeGroupes::afficherGroupesMusicien(const string& nomMusicien) const
{
	//TODO: Utiliser votre fonction pour trouver le musicien (au lieu de le mettre à nullptr).
	const Musicien* musicien = trouverMusicien(nomMusicien);
	if (musicien == nullptr)
		cout << "Aucun musicien de ce nom" << endl;
	else
		musicien->joueDans.afficher();
}


int ListeGroupes::size() const
{
	return nElements_;
}


Groupe* ListeGroupes::operator[](int index) const
{
	return elements_[index];
}


int main()
{
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	//int* fuite = new int; //TODO: Enlever cette ligne après avoir vérifié qu'il y a bien un "Fuite detectee" de "4 octets" affiché à la fin de l'exécution, qui réfère à cette ligne du programme.

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	//TODO: Chaque TODO dans cette fonction devrait se faire en 1 ou 2 lignes, en appelant les fonctions écrites.

	//TODO: La ligne suivante devrait lire le fichier binaire en allouant la mémoire nécessaire.  Devrait afficher les noms des 22 musiciens sans doublons (par l'affichage pour fins de débogage dans votre fonction lireMusicien).
	ListeGroupes listeGroupes("groupes.bin");

	cout << ligneDeSeparation << "Le premier groupe de la liste est:" << endl;
	//TODO: Afficher le premier groupe de la liste.  Devrait être Nirvana.
	afficherGroupe(*listeGroupes[0]);

	cout << ligneDeSeparation << "Les groupes sont:" << endl;

	//TODO: Afficher la liste des groupes.  Il devrait y en avoir 7.
	listeGroupes.afficher();

	//TODO: Modifier l'année de naissance de Dave Grohl pour être 1969 (a une valeur qui n'a pas de sens dans les données lues du fichier).  Vous ne pouvez pas supposer l'ordre des groupes et des membres dans les listes, il faut y aller par son nom.
	Musicien* daveGrohl =
		listeGroupes.trouverMusicien("Dave Grohl");

	daveGrohl->anneeNaissance = 1969;

	cout << ligneDeSeparation << "Liste des groupes où Dave Grohl joue sont:" << endl;

	//TODO: Afficher la liste des groupe où Dave Grohl joue.  Il devrait y avoir Nirvana et Foo Fighters.
	listeGroupes.afficherGroupesMusicien("Dave Grohl");


	//TODO: Détruire et enlever le premier groupe de la liste (Nirvana).  Ceci devrait "automatiquement" (par ce que font vos fonctions) détruire les membres Kurt Cobain et Krist Novoselic, mais pas Dave Grohl puisqu'il joue aussi dans Foo Fighters.
	Groupe* groupeADetruire = listeGroupes[0];

	listeGroupes.enleverGroupe(groupeADetruire);
	detruireGroupe(groupeADetruire);

	cout << ligneDeSeparation << "Les groupes sont maintenant:" << endl;

	//TODO: Afficher la liste des groupes.
	listeGroupes.afficher();

	//TODO: Faire les appels qui manquent pour avoir 0% de lignes non exécutées dans le programme (aucune ligne rouge dans la couverture de code; c'est normal que les lignes de "new" et "delete" soient jaunes).  Vous avez aussi le droit d'effacer les lignes du programmes qui ne sont pas exécutée, si finalement vous pensez qu'elle ne sont pas utiles.
	listeGroupes.afficherGroupesMusicien("Musicien inexistant");

	//TODO: Détruire tout avant de terminer le programme.  La bibliothèque de verification_allocation devrait afficher "Aucune fuite detectee." a la sortie du programme; il affichera "Fuite detectee:" avec la liste des blocs, s'il manque des delete.
	listeGroupes.detruire();
}