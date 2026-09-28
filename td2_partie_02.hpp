/**
* Déclaration de la classe ListeGroupes et des structures représentant une
* collection de groupes musicaux en mémoire.
* \file td2_partie_02.hpp
* \author Cedric KIENGO && Yorrick CABO
* \date 27 septembre 2026
*/

#pragma once

#include <span>
#include <string>

struct Groupe;
struct Musicien;

class ListeGroupes
{
public:
	ListeGroupes();
	explicit ListeGroupes(int capaciteInitiale);

	int obtenirNElements() const;
	std::span<Groupe* const> obtenirGroupes() const;

	void ajouterGroupe(Groupe* groupe);
	void retirerGroupe(Groupe* groupe);
	Musicien* trouverMusicien(const std::string& nom) const;
	void liberer();

private:
	int capacite_;
	int nElements_;
	Groupe** elements_;
};

struct ListeMusiciens
{
	int capacite, nElements;
	Musicien** elements;
};

struct Groupe
{
	std::string nom, genre;
	int anneeFormation;
	ListeMusiciens membres;
};

struct Musicien
{
	std::string  nom, pays;
	int anneeNaissance;
	ListeGroupes joueDans;
};