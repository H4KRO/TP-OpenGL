#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <semaphore.h>
#include <errno.h>
#include <GL/glut.h>
#include <time.h>

sem_t sem_generator;

int x_fen = 800;
int y_fen = 600;

typedef struct Ball Ball;
struct Ball {
  float x;
  float y;
  float z;
  double rayon;
  double x_vector;
  double y_vector;
  float no_mat[4];
  float mat_diffuse[4];
};

const int nOfBalls = 40;
int ballCursor = 0;
Ball balls[40];


int slice = 10;
int slacks = 10;

double speed = .4;
double g = 1.07;

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

  for(int i = 0; i < nOfBalls; i++) {
    float mat_diffuse[] = {0.0f, 0.7f, 0.3f, 1.0f};
    float mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float low_shininess = 5.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, balls[i].no_mat);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, balls[i].mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, low_shininess);

    glPushMatrix(); // Sauvegarde de la matrice
    glTranslatef(balls[i].x, balls[i].y, balls[i].z); //on déplace la matrice en x y z
    glutSolidSphere(balls[i].rayon , slice , slacks); //on dessine une sphere
    glPopMatrix(); //On revient à la matrice sauvegardée
  }

  glutSwapBuffers() ; // affiche à l’écran le buffer dans lequel nous avons dessiné
}

void *update_f() {
  while(1 == 1){
    for(int i = 0; i < nOfBalls; i++) {
      balls[i].x += balls[i].x_vector * speed;
      balls[i].y += balls[i].y_vector * speed;
      if(balls[i].y_vector > 0 && balls[i].y_vector < 0.01){
        balls[i].y_vector = -balls[i].y_vector;
      }else if(balls[i].y_vector > 0){
        balls[i].y_vector = balls[i].y_vector / g;
      }else if(balls[i].y_vector < 0){
        balls[i].y_vector = balls[i].y_vector * g;
      }
      balls[i].x_vector = balls[i].x_vector * .999 ;
      if(balls[i].y < -8) {
        balls[i].y_vector *= -1;
      }
    }
    if(balls[0].x > 7) {
      sem_post(&sem_generator);
    }
    usleep(15000);
  }
}

double randomFloat(float min, float max) {
  double r = (double) rand() / (double) RAND_MAX;
  return(min + r * (max - min));
}

void *thread_generator (void *arguments) {
  while(1 == 1) {
    sem_wait(&sem_generator);
    balls[ballCursor].x = randomFloat(-20, -17);
    balls[ballCursor].y = randomFloat(1, 5);
    balls[ballCursor].z = randomFloat(-20, -15);
    balls[ballCursor].rayon = randomFloat(.5, .8);
    balls[ballCursor].x_vector = .12;
    balls[ballCursor].y_vector = .7;
    balls[ballCursor].no_mat[0] = randomFloat(0, 1);
    balls[ballCursor].no_mat[1] = randomFloat(0, 1);
    balls[ballCursor].no_mat[2] = randomFloat(0, 1);
    balls[ballCursor].no_mat[3] = randomFloat(0, 1);
    balls[ballCursor].mat_diffuse[0] = balls[ballCursor].no_mat[0];
    balls[ballCursor].mat_diffuse[1] = balls[ballCursor].no_mat[1];
    balls[ballCursor].mat_diffuse[2] = balls[ballCursor].no_mat[2];
    balls[ballCursor].mat_diffuse[3] = balls[ballCursor].no_mat[3];
    ballCursor += 1;
    if(ballCursor >= nOfBalls) {
      ballCursor = 0;
    }
    usleep(600000);
  }
}

int main() {
  srand( time( NULL ) );

  sem_init(&sem_generator, 0, nOfBalls);
  pthread_t generator;
  pthread_create(&generator, NULL, thread_generator, NULL);

  pthread_t affiche;
  pthread_create(&affiche, NULL, thread_affichage, NULL);
  pthread_t update;
  pthread_create(&update, NULL, update_f, NULL);
  pthread_join(affiche, NULL);
  pthread_join(update, NULL);
}
