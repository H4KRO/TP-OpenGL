#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <semaphore.h>
#include <errno.h>
#include <GL/glut.h>

int x_fen = 800;
int y_fen = 600;

float x = -10;
float y = 0;
float z = -10;

double rayon = .5;
int slice = 10;
int slacks = 10;

double speed = .4;
double x_vector = .08;
double y_vector = .8;
double g = 1.05;

void display();

void changeSize(int w, int h) {
  if(h == 0) //Evite la division par zéro
  h = 1;

  float ratio =  w * 1.0 / h; //calcul du ratio de la rentre
  glMatrixMode(GL_PROJECTION); //Utilise la matrice de projection
  glLoadIdentity();   //Remise à zéro de la matrice

  // Set the viewport to be the entire window
  glViewport(0, 0, w, h);

  // Set the correct perspective.
  gluPerspective(80,ratio,1,100);

  // Get Back to the Modelview
  glMatrixMode(GL_MODELVIEW);
}

void affiche()
{
  glutPostRedisplay() ;
  usleep(10000) ;
}

void *thread_affichage (void *arguments)
{
  int nbarg=1;

  float ratio = 1.0* x_fen / y_fen;  //Calcul de ratio de la fenêtre

  glutInit(&nbarg,(char**)arguments); //Initialisation de Glut
  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB); //Utilise le codage RBG en double buffer
  glutInitWindowSize(x_fen,y_fen);  // taille de la fenêtre que l’on va créer
  glutInitWindowPosition(100,100);  // position de la fenêtre sur l’écran
  glutCreateWindow("Bonjour"); //Non de la fenêtre

  glClearColor(0.0,0.0,0.0,0.0);

  glEnable(GL_DEPTH_TEST); //On active le buffer de profondeur
  glShadeModel(GL_SMOOTH); //On choisit un ombrage doux
  glEnable(GL_LIGHTING);	 //On active les lumières

  glEnable(GL_LIGHT0);   //On active une lampe
  glMatrixMode(GL_PROJECTION); //Utilise la matrice de projection

  glutDisplayFunc(display);  // défini la fonction appelée pour afficher la scène
  glutReshapeFunc(changeSize); // défini la fonction lancée lorsque le système l’on redimensionne la fenêtre
  glutIdleFunc(affiche); // défini la fonction lancée lorsque le système n’a plus rien a faire
  //glutSpecialFunc(GestionSpecial); // défini la fonction lancée lorsque l’on appuie sur une touche spéciale
  //glutKeyboardFunc(keyboard); /// défini la fonction lancée lorsque l’on appuie sur une touche classique du clavier

  glutMainLoop(); // fonction bloquante permettant la gestion de l’affichage
}


void display ()
{
  glClear (GL_COLOR_BUFFER_BIT) ; // efface le buffer
  //Exemple de réglages de la lumière GL_LIGHT0
  float ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
  float diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
  float specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
  float g_lightPos[4] = { 10, 10, -200, 1 };

  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  glLightfv(GL_LIGHT0, GL_POSITION, g_lightPos);
  //Exemple de définition d’un matériau
  float no_mat[] = {0.0f, 0.0f, 0.0f, 1.0f};
  float mat_diffuse[] = {0.0f, 0.7f, 0.3f, 1.0f};
  float mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
  float low_shininess = 5.0f;
  glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, low_shininess);

  glPushMatrix(); // Sauvegarde de la matrice

  glTranslatef(x, y, z); //on déplace la matrice en x y z
  glutSolidSphere(rayon , slice , slacks); //on dessine une sphere

  glPopMatrix(); //On revient à la matrice sauvegardée

  glutSwapBuffers() ; // affiche à l’écran le buffer dans lequel nous avons dessiné
}

void *update_f() {
  sleep(2);
  while(1 == 1){
    x += x_vector * speed;
    y += y_vector * speed;
    if(y_vector > 0 && y_vector < 0.01){
      y_vector = -y_vector;
    }else if(y_vector > 0){
      y_vector = y_vector / g;
    }else if(y_vector < 0){
      y_vector = y_vector * g;
    }
    x_vector = x_vector * .999 ;
    if(y < 0) {
      y_vector = -( y_vector * .6);
    }
    usleep(15000);
  }
}

int main() {
  pthread_t affiche;
  pthread_create(&affiche, NULL, thread_affichage, NULL);
  pthread_t update;
  pthread_create(&update, NULL, update_f, NULL);
  pthread_join(affiche, NULL);
  pthread_join(update, NULL);
}
