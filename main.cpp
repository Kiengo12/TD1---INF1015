/*******************************************************************************
 * INF1015 - Programmation orientee objet avancee
 * TD 1
 * Auteurs: Kiengo Cedric - 
 ******************************************************************************/

#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <iomanip>

using namespace std;

const int TAILLE_MAX = 100;
const int PLUS_PETIT_ENTIER = 2;


// ---------------------------------------------------------------------------
// Exercice 1 : Somme reelle
// ---------------------------------------------------------------------------

double calculerSomme(int n){
    double somme = 0.0;
    for (int i = 1; i <= n; i++)
    {
        somme += 1.0 / i;
    }
    return somme;
}

void sommeReelle(){
    int n = 0;
    cout << "Entrez un entier n : ";
    cin >> n;

    double sommeN = calculerSomme(n);
    int plusPetitEntier = static_cast<int>(ceil(sommeN));

    cout << "Le plus petit entier a tel que Sn <= a est : " << plusPetitEntier << endl;
}


// ---------------------------------------------------------------------------
// Exercice 2 : Diviseurs d'un nombre
// ---------------------------------------------------------------------------

void diviseursNombre(){
    bool validation = true;
    int nDiviseurs = 0;
    int nombre = 0;

    do{
        cout << "Entrez un nombre superieur ou egal a 2 : ";
        cin >> nombre;
        if(nombre >= 2){
            validation = false;
        }
    } while (validation);

    for (int i = 2; i < nombre; i++){
        if (nombre % i == 0){
            if (nDiviseurs == 0){
                cout << "Pour n= "<< nombre <<": Le nombre " << nombre << " n'est pas premier." << endl;
                cout << "\tLes diviseurs de " << nombre << " sont : ";
            }else{
                cout << ",";
            }

            cout << i;
            nDiviseurs++;
        }
    }

    if (nDiviseurs == 0){
        cout << "Le nombre " << nombre << " est premier." << endl;
    }else{
        cout << endl;
    }
}


// ---------------------------------------------------------------------------
// Exercice 3 : Rotation d'une chaine de caractere
// ---------------------------------------------------------------------------

string rotationChaine(string chaine, int n, bool versLaDroite){
    if (versLaDroite){
        chaine = chaine.substr(chaine.length() - n) + chaine.substr(0, chaine.length() - n);
    }
    else{
        chaine = chaine.substr(n) + chaine.substr(0, n);
    }

    return chaine;
}

void rotationChaineDeCaractere(){
    string chaine;
    int n = 0;
    bool versLaDroite = false;

    cout << "Entrez une chaine de caractere : ";
    cin >> chaine;

    cout << "Entrez un nombre n entre 1 et " << chaine.length() << " : ";
    cin >> n;

    cout << "Rotation vers la droite (1) ou vers la gauche (0) : ";
    cin >> versLaDroite;

    chaine = rotationChaine(chaine, n, versLaDroite);
    cout << "Resultat : " << chaine << endl;
}


// ---------------------------------------------------------------------------
// Exercice 4 : Devinette
// ---------------------------------------------------------------------------

void devinette(){
    int nombreCache = 0;
    int nombreDevine = 0;
    int nTentatives = 0;

    cout << "Utilisateur 1, entrez le nombre a deviner : ";
    cin >> nombreCache;

    cout << "Utilisateur 2, proposez un nombre : ";
    cin >> nombreDevine;
    nTentatives++;

    while (nombreDevine != nombreCache){
        if (nombreDevine < nombreCache){
            cout << "Trop petit, reessayez : ";
        }
        else{
            cout << "Trop grand, reessayez : ";
        }

        cin >> nombreDevine;
        nTentatives++;
    }

    cout << "Victoire !" << endl;
    cout << "Vous avez trouve le nombre en " << nTentatives << " tentatives." << endl;
}


// ---------------------------------------------------------------------------
// Exercice 5 : Doublons
// ---------------------------------------------------------------------------

// Decale les elements du tableau d'un cran vers la gauche a partir de
// l'indice indiceASupprimer, et retourne la nouvelle taille du tableau.
int retirerElement(int nombres[TAILLE_MAX], int nElements, int indiceASupprimer)
{
    for (int i = indiceASupprimer; i < nElements - 1; i++){
        nombres[i] = nombres[i + 1];
    }
    return nElements - 1;
}

// Supprime les doublons d'un tableau (ne fait ni lecture clavier ni
// affichage) et retourne le nombre de valeurs restantes.
int supprimerDoublons(int nombres[TAILLE_MAX], int nElements)
{
    for (int i = 0; i < nElements; i++){
        for (int j = i + 1; j < nElements; j++){
            if (nombres[i] == nombres[j]){
                nElements = retirerElement(nombres, nElements, j);
                j--;
            }
        }
    }
    return nElements;
}

void afficherTableau(const int nombres[TAILLE_MAX], int nElements){
    cout << "[";
    for (int i = 0; i < nElements; i++){
        cout << nombres[i];
        if (i < nElements - 1){
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

void doublons(){
    int nElements = 0;
    int nombres[TAILLE_MAX];

    cout << "Entrez la taille n de votre tableau : ";
    cin >> nElements;

    cout << "Entrez les " << nElements << " valeurs de votre tableau : " << endl;
    for (int i = 0; i < nElements; i++){
        cout << "Veuillez entrer le nombre numero " << i + 1 << endl;  
        cin >> nombres[i];
    }

    cout << "Tableau saisi : ";
    afficherTableau(nombres, nElements);

    nElements = supprimerDoublons(nombres, nElements);

    cout << "Tableau sans doublons : ";
    afficherTableau(nombres, nElements);
}


// ---------------------------------------------------------------------------
// Exercice 6 : Evaluation cours
// ---------------------------------------------------------------------------

struct Etudiant{
    int matricule = 0;
    double noteIntra = 0.0;
    double moyLabs = 0.0;
    double noteFinal = 0.0;
    double moyenneFinale = 0.0;
};

Etudiant trouverMeilleureMoyenneLabs(const Etudiant etudiants[TAILLE_MAX], int nEtudiants){
    Etudiant meilleur = etudiants[0];
    for (int i = 1; i < nEtudiants; i++){
        if (etudiants[i].moyLabs > meilleur.moyLabs)
        {
            meilleur = etudiants[i];
        }
    }
    return meilleur;
}

Etudiant trouverMeilleureMoyenneFinale(const Etudiant etudiants[TAILLE_MAX], int nEtudiants){
    Etudiant meilleur = etudiants[0];
    for (int i = 1; i < nEtudiants; i++){
        if (etudiants[i].moyenneFinale > meilleur.moyenneFinale){
            meilleur = etudiants[i];
        }
    }
    return meilleur;
}

void evaluationCours(){
    Etudiant etudiants[TAILLE_MAX];
    int nEtudiants = 0;
    double sommeMoyennes = 0.0;

    int matricule = 0;
    double noteIntra = 0.0;
    double moyLabs = 0.0;
    double noteFinal = 0.0;

    ifstream fichierNotes("notes.txt");

    if (!fichierNotes.is_open()){
        cout << "Impossible d'ouvrir le fichier notes.txt" << endl;
        return;
    }

    while (fichierNotes >> matricule >> noteIntra >> moyLabs >> noteFinal){
        etudiants[nEtudiants].matricule = matricule;
        etudiants[nEtudiants].noteIntra = noteIntra;
        etudiants[nEtudiants].moyLabs = moyLabs;
        etudiants[nEtudiants].noteFinal = noteFinal;
        etudiants[nEtudiants].moyenneFinale = 0.3 * noteIntra + 0.3 * moyLabs + 0.4 * noteFinal;

        sommeMoyennes += etudiants[nEtudiants].moyenneFinale;
        nEtudiants++;
    }
    fichierNotes.close();

    if (nEtudiants == 0){
        cout << "Aucune donnee trouvee dans notes.txt" << endl;
        return;
    }

    double moyenneClasse = sommeMoyennes / nEtudiants;
    Etudiant meilleurLabs = trouverMeilleureMoyenneLabs(etudiants, nEtudiants);
    Etudiant meilleurFinal = trouverMeilleureMoyenneFinale(etudiants, nEtudiants);

    ofstream fichierEvaluation("evaluation.txt");
    if (!fichierEvaluation.is_open()){
        cout << "Impossible de creer le fichier evaluation.txt" << endl;
        return;
    }

    fichierEvaluation << fixed << setprecision(2);
    fichierEvaluation << "La moyenne de la classe est : " << moyenneClasse << endl;
    fichierEvaluation << "L'etudiant ayant le matricule " << meilleurLabs.matricule
        << " a la meilleure note " << meilleurLabs.moyLabs << " dans les labs" << endl;
    fichierEvaluation << "L'etudiant ayant le matricule " << meilleurFinal.matricule
        << " a la meilleure moyenne " << meilleurFinal.moyenneFinale << endl;

    fichierEvaluation.close();

    cout << "Resultats ecrits dans evaluation.txt" << endl;
}


int main(){
    //sommeReelle();
    //diviseursNombre();
    //rotationChaineDeCaractere();
    //devinette();
    //doublons();
    evaluationCours();


    return 0;
}