
/****************************************************************************
********* PROGRAM FOR SOLVING THE MATRIX EQUATIONS FOR THE GIVEN CIRCUIT ****
****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h>

#define MAXBUF 512
#define MAXNODES 32


// Defining the structures.
	
typedef struct Elem {
  struct Elem *prev;
  struct Elem *next;
  int n1;
  int n2;
  int *n3;
  int *n4;
  char *depname;
  char *name;
  complex *value;
} Elem;

typedef struct List {
  struct Elem *head;
  int count;
} List;


// Function Prototypes
int main (int argc, char **argv);
int getval (complex *result, char *str);
void freelist (Elem *head);
void getnode (Elem *head, int *nodelist);
void diagfix (int size, float (*g)[size], float curr[]);
void solve (int d, int e, Elem * new, int mode, complex * value);


// Declaring the global variables which will be used by function to calculate the solution

int nodetbl[99];
int i, j, acflag = 0;
int voltindex[20][4];
char whatever;
float w;


// *************************
// ***** MAIN FUNCTION *****
// *************************


int main (int argc, char **argv) {
  
  // Checking whether the file parameter is passed in command line argument
  if (argc != 2) {
    printf ("Usage: ./myspice <filename>\n");
    exit (1);
  }
  
  
  // Checking whether the passed file parameter is accesible
  FILE *fp = fopen (argv[1], "r");
  if (fp == NULL) {
    printf("File invalid.\n");
    exit (2);
  }
  
  
  // Allocating the memory to the variables used during the program.
  List *list = malloc (sizeof (List));
  list->count=0;
  list->head = NULL;


  // *xbuf is used for the purpose of freeing the memory allocated to buf after it's used.
  char *buf = malloc (MAXBUF*sizeof(char));
  char *xbuf= NULL;
  char *tok = malloc (16*sizeof(tok));
  Elem *new = (Elem*) malloc (sizeof(Elem));
  Elem *old = NULL;
  int e = 0;


  //Initializing the voltindex array. This is done to distinguish the voltage sources present from those which are not.
  for (i=0; i<20; i++)
    for (j=0; j<4; j++)
      voltindex[i][j] = -1;
  i=0;	


  // The while loop takes input from the given file till all the lines are successfully read.
  while (fgets(buf, MAXBUF, fp)) {
    
    // This is the condition for discarding empty lines or lines starting with '*' (comments).
    if (buf[0] == '*'|| buf[0]== '\0'|| buf[0]=='\n')
      continue;
    
    // Creating the linked list. No next element will be present to the entry made earliest.
    if (list->count == 0) {
      list->head = new;
      new->next = NULL;
      new->prev = NULL;
    }

    else {
      list->head = new;
      new->next = old;
      old->prev = new;
      new->prev = NULL;
    }
		
    tok = strtok (buf, " \t");
    xbuf = buf;
    buf = NULL;


   		
    // The switch statement is used for making cases according to the element    
    
    switch (whatever = tok[0]) {
    
      /*For voltage source, we create an array called voltindex to tabulate the voltage source name, n1, n2 and its row number in G matrix.
	This will ensure we have access to all this whenever required (mostly in case of a dependent source).*/

    case 'L':
    case 'l':
    case 'C':
    case 'c': acflag = 1; // Flag to indicate presence of L or C in circuit.

    case 'V':
    case 'v': sscanf(tok, "%c%d", &whatever,&voltindex[i][0]); e++;
    case 'I':
    case 'i':
    case 'R':
    case 'r': {
      new->name = malloc (5*sizeof(char));
      strcpy (new->name, tok);
      new->n1 = atoi (strtok (buf, " \t"));
      new->n2 = atoi (strtok (buf, " \t"));
      new->n3 = NULL;
      new->n4 = NULL;
      new->depname = NULL;
      

      /*Name of the source is stored in voltindex[][0].
	n1 is stored in voltindex[][1].
	n2 is stored in voltindex[][2].
	Later, when we create G matrix, row number is stored in voltindex[][3].*/

      if (tok[0] == 'v' || tok[0] == 'V') {
      	voltindex[i][1] = new->n1;
	voltindex[i++][2] = new->n2;
      }
      break;
    }
    

      // For each of dependent sources and voltage sources, an extra line is added in the G matrix.
      // Hence, we increase the variable e by 1 to keep count of them.
    case 'E':
    case 'e':
    case 'G':
    case 'g': {
      new->name = malloc (5*sizeof(char));
      strcpy (new->name, tok);
      new->n1 = atoi (strtok (buf, " \t"));
      new->n2 = atoi (strtok (buf, " \t"));
      new->n3 = malloc(sizeof(int));
      new->n4 = malloc(sizeof(int));
      *new->n3 = atoi (strtok (buf, " \t"));
      *new->n4 = atoi (strtok (buf, " \t"));
      new->depname = NULL;
      e++;
      break;
    }
    
    
    case 'F':
    case 'f':
    case 'H':
    case 'h': {
      new->name = malloc (5*sizeof(char));
      strcpy (new->name, tok);
      new->n1 = atoi (strtok (buf, " \t"));
      new->n2 = atoi (strtok (buf, " \t"));
      new->n3 = NULL;
      new->n4 = NULL;
      new->depname = malloc (5*sizeof(char));
      strcpy (new->depname, strtok (buf, " \t"));
      e ++;
      break;
    }


      /* If some error occures while reading the data, the linked list and
	 other allocations are freed and the program stops.*/
    default : {
      printf("Syntax of the spice netlist is incorrect. Unknown element : %s\n", tok);
      freelist (list->head);
      free (buf);
      free (list);
      free (buf);
      free (tok);
      fclose(fp);
      exit(3);
    }
    }
    

    // Function getval returns 1 if value conversion was successful. Else, it returns 0.
    new->value = malloc (sizeof(float));

      if (getval (new->value, strtok (buf, " \t")))
	;
    
      // If error occures in reading the value, the function frees up the allocated memory and stops.
      else {
	printf("\nFor %s, syntax of value is incorrect.\n", new->name);
	freelist (list->head);
	free (list);
	free (buf);
	free (tok);
	free (new->value);
	exit(4);
      }
  
    // Freeing the xbuf (previous mem location of buf) everytime the loop reaches end.
    free (xbuf);
   
    buf = malloc (MAXBUF*sizeof(char));
    list->count += 1;
    old = new;
    new = NULL;
    new = malloc (sizeof(Elem));
  }
  free (new);
  
  
  /*Determining the number of distinct nodes, creating the table assigning node number to the name and storing the information in the nodes. For doing this, we first create an array named nodelist. Array will be initialized to zero at first. It will read in the values of n1 and n2 from each structure in linked list. Before adding, it will be checked for repetition. Then, a name will be assigned to it.*/
  
  int *nodelist= malloc(99*sizeof(int));
  for (i=0; i<99; i++)
    nodelist [i] = 0;
  
  int d;
  getnode (list->head, nodelist);
  

  // Initializing the array "node table" used for distinct node detection
  for (i=0, d=0; i<99; i++) {
    if (nodelist[i] != 0)
      nodetbl[i] = d++;
    else
      nodetbl[i] = -1;
  }
  
  
  // Number of nodes and number of additional lines required in G are printed here.
  printf("\nThe given circuit has %d distinct nodes and %d additional equations due to voltage/dependent sources.\n\n", d, e);



  // Voltage sources present in circuit and Format of input line is printed out to minimize the error by user.
  // Still, the error is notified if input is incorrect.

  printf("Voltage source(s) connected: ");
  for (i=0; i<20; i++){
    if (voltindex[i][0] != -1)
      printf("v%d ", voltindex[i][0]);
  }


  // Prompt for user to give the input line.
  printf("\n\nNow provide the input line in format:\n\n'dc v# vmin vmax step' or \n'ac v# fmin fmax n'.\n\n");


  /* Declaring the variables to read the input line by user.
     Here, all character arrays will first store the input in string pieces.
     Then, it will be checked for its correctness.
     Then, tokens will be appropriately converted.
  */
  
  char srctyp1[3], vsrc1[4], min1[6], max1[6], step1[6];
  
  int srctyp, vsrc;
  complex min, max, step;
  
  
  
  // Taking input line from user. Stored in tokens.
  scanf("%s %s %s %s %s", srctyp1, vsrc1, min1,  max1, step1);
  

  // Checkig the first token: (AC/DC)
  if (strcmp(srctyp1, "ac")==0)
    srctyp = 2;
  else if (strcmp(srctyp1, "dc")==0)
    srctyp = 1;
  else {
    printf("Input line format incorrect! Please write 'ac' or 'dc'.\n");
    exit(10);
  }


  // Checking the second token: (Voltage source)
  sscanf(vsrc1, "%c%d", &whatever, &vsrc);
  if (whatever == 'v' || whatever == 'V')
    for (i=0; i<20; i++)
      if (voltindex[i][0] == vsrc)
	break;

  // If mentioned voltage source is not present:
  if (i == 20) {
    printf("Incorrect voltage source!\n");
    exit(11);
  }


  // Checking and converting the next three tokens: (max, min and step)
  getval(&min, min1);
  getval(&max, max1);
  getval (&step, step1);



  /* Next part of the function depends upon the type of the source. Here, we read the input line and see for its correctness. Also, we rule out the cases where DC analysis is carried out for circuit with inductors and capacitors. The user is shown the error and function exits if DC circuit contains memory elements. srctyp = 1 means 'dc' and srctyp = 2 means 'ac'.*/

  int p;  
  switch (srctyp) {

 // DC case:
  case 1: {
    new = list->head;

    // Traversing the linked list to reach the desired voltage source.
    while (new != NULL) {
      old = new;
      new = new->next;
      if (strcmp(old->name, "vsrc1")==0)
	break;
    }

    // This checks whether L or C is present in the given DC circuit.
    if (acflag == 1) {
      printf("\nDC analysis is not possible when L or C are present.\nRemove them & try again.\n\n");
      exit(13);
    }
    

    // This part modifies the source voltage magnitude in steps and solves the circuit each time.
    for (p=0; p< ( (int) ((max-min)/step) +1); p++) {
      *old->value = min + p*step;
      solve (d, e, list->head, srctyp, old->value);
    }
   break;
  }


    // AC Case: 
  case 2: {

    // Traversing the list to reach desired source.
    new = list->head;
    while (new != NULL) {
      old = new;
      new = new->next;
      if (strcmp(old->name, "vsrc1")==0)
	break;
    }
    
    
    // Changing the value of the source as required by the function and solving the circuit.
    for (p=0; p < (int) (log((max/min))*step/2.303 +1); p++) {
      w = pow (10, (log(min) + p*2.303/step));
      solve (d, e, list->head, srctyp, old->value);
    }
   break;

  }
  }


  // Instructions for handling the output file for GNUPLOT.  

  printf("\nThe output has been successfully written to the file 'spice.out'.\n");
  printf("\nIn case of DC,\n1. The 1st column in the output corresponds to the mentioned voltage source.\n");
  printf("2. The next %d columns of the output correspond to the %d node voltages.\n", d, d);
  printf("3. The next %d columns correspond to the currents through the voltage sources.\n", e);
  printf("   The order of these currents is same as order of the sources in the netlist.\n");
  printf("\nIn case of AC,\n1. The first column in the output corresponds to the voltage source which is varied.\n");
  printf("2. The next format is same as that of DC; except that every magnitude column is followed by a phase column.\n");
  printf("   Thus, there twice the number of columns here. The phase given is in degrees.\n\n");
  printf("\nThank you.\n");

} 


// ******************************
// ***** THE SOLVE FUNCTION *****
// ******************************

void solve (int d, int e, Elem * head, int mode, complex *value) {
  
  // Creating the G matrix "g", unknown matrix "v" and current matrix "curr".
  // Size of matrices is ( # nodes + # additional equations).

  complex g[d+e][d+e], v[d+e], curr[d+e];
  int number = d;

  // Initializing the matrices
  for (i=0; i<d+e; i++) {
    v[i] = 0;
    curr[i] = 0;
    for (j=0; j<d+e; j++)
      g[i][j] = 0;
  }

  Elem * old = NULL, *new = NULL;

  /* Linked list is traversed. First while loop analyzes the netlist for resistance and independent sources.
     This is necessary because of the indexing of voltage sources should be done before a dependent source.*/
 
  new = head;
  
  while (new !=NULL) {
    
    old = new;
    new = new->next;
    
    // Standard MNA algorithm is followed here for filling the matrix elements.
    switch (old->name[0]) {
    
    case 'l':
    case 'L': {
      *old->value = I*w*(*old->value);
    }
    case 'C':
    case 'c': {
      if (old->name[0] == 'c' || old->name[0] == 'C')
	*old->value = -I/w/(*old->value);
    }
    case 'r': 
    case 'R': {
      g[ nodetbl[old->n1] ][ nodetbl [old->n1] ] += 1/(*old->value);
      g[ nodetbl[old->n1] ][ nodetbl [old->n2] ] += -1/(*old->value);
      g[ nodetbl[old->n2] ][ nodetbl [old->n1] ] += -1/(*old->value);
      g[ nodetbl[old->n2] ][ nodetbl [old->n2] ] += 1/(*old->value);
      break;
    }

    case 'V':
    case 'v': {
      
      g[d][ nodetbl[old->n2] ] += 1;
      g[d][ nodetbl[old->n1] ] += -1;
      curr[d] += (*old->value);
      g[ nodetbl[old->n2] ][d] += -1;
      g[ nodetbl[old->n1] ][d] += 1;
      
      // The voltindex[][3] corresponds to the row number of this voltage source in G matrix.
      sscanf(old->name, "%c%d", &whatever, &i);
      for (j=0; j<20; j++) {
	if (voltindex[j][0] == i)
	  break;
      } 
      voltindex[j][3] = d;
      
      // Next element will be filled in next row.
      d++;
      break;
    }
    case 'I':
    case 'i': {
      curr[ nodetbl[old->n2] ] += *old->value;
      curr[ nodetbl[old->n1] ] += -(*old->value);
      break;
    }
      old = NULL;
    }
  }


  // This second while loop makes changes in G matrix because of dependent sources.
  
  new = head;
  
  while (new !=NULL) {

    old = new;
    new = new->next;
      
    switch (old->name[0]) {
    
    case 'E': 
    case 'e': {
      g[d][ nodetbl[old->n1] ] += -1;
      g[d][ nodetbl[old->n2] ] += 1;
      g[d][ nodetbl[*old->n3] ] += *old->value;
      g[d][ nodetbl[*old->n4] ] += -(*old->value);
      g[ nodetbl[old->n2] ][d] += -1;
      g[ nodetbl[old->n1] ][d] += 1;
      d++;
      break;
    }
    
    case 'G': 
    case 'g': {
      g[d][ nodetbl[*old->n4] ] += 1;
      g[d][ nodetbl[*old->n3] ] += -1;
      g[ nodetbl[old->n1] ][d] += 1;
      g[ nodetbl[old->n2] ][d] += -1;
      g[d][d] = -1/(*old->value);
      d++;
      break;
    }
    
    case 'F': 
    case 'f': {
      
      // Checking whether the name of dependent source is valid.
      sscanf(old->depname, "%c%d", &whatever, &j);
      for (i=0; i<20; i++) {
      	if (voltindex[i][0] == j)
	  break;
      }
      // If no such voltage source is found, program exits with an error.
      if (voltindex[i][0] != j) {
	printf("Depname not valid.\n");
	exit(5);
      }
	
      g [ old->n2 ][ voltindex[i][3] ] -= *old->value;
      g [ old->n1 ][ voltindex[i][3] ] += *old->value;
      g [d][ voltindex[i][3] ] += *old->value;
      g [d][d] -= 1;
	
      d++;      
      break;
    }
	
    case 'H': 
    case 'h': {
      
      // Checking whether the name of dependent source is valid.
      sscanf(old->depname, "%c%d", &whatever, &j);
      for (i=0; i<20; i++) {
	if (voltindex[i][0] == j)
	  break;
      }
      // If no such voltage source is found, program exits with an error.
      if (voltindex[i][0] != j) {
	printf("Depname not valid.\n");
	exit(5);
      }
	
      g[d][ nodetbl[old->n2] ] += 1;
      g[d][ nodetbl[old->n1] ] += -1;
      g[d][ voltindex[i][3] ] -= *(old->value);
      g[ nodetbl[old->n2] ][d] += -1;
      g[ nodetbl[old->n1] ][d] += 1;
      d++;
      break;
    }
    }
    old = NULL;
  }    


  // ************************************************************************************
  // ***** Converting the matrix to upper triangular matrix using Gaussian Elimination **
  // ************************************************************************************
  
  // The algorithm is standard gaussian elimination.

  float scale;
  int k;

  /* Pivoting is done to ensure that cases with diagonal element absent are also solved.
     (It was optional, but I did it for the sake of completeness) */

  for (k=1; k<number+e; k++) {
    if (g[k][k] == 0) {
      for (i=k+1; i<number+e; i++) {
	if (g[i][k] != 0) {
	  for (j=0; j<number+e; j++)
	    g[k][j] = g[k][j] + g[i][j];
	  curr[k] = curr[k] + curr[i];
	  break;
	}
      }
    }
    
    if (g[k][k] == 0)
      continue;

    for (i=k+1; i<number+e; i++) {
      scale = g[i][k]/g[k][k];
      for (j=0; j<number+e; j++)
	g[i][j] = g[i][j] - scale*g[k][j];
      curr[i] = curr[i] - scale*curr[k];
    }
  }
  

  // Solving the Equations. Pivoting ensures that the matrix can be solved in ALL cases.

  for (k= number+e-1; k>=0; k--) {
    v[k] = curr[k];
    if (k != number+e-1)
      for (j=k+1; j<number+e; j++)
	v[k] -= g[k][j] * v[j];
    v[k] = v[k] / g[k][k];
  }


  // Printing the solutions of unknown matrix to the file spice.out.

  // File is overwritten if it is already present.
  // File is created if it isn't already present.

  FILE *fp;
  fp = fopen("spice.out", "w");
  fp = fopen("spice.out", "a+");


  // In case of DC:
  if (mode == 1) {
    fprintf(fp, "%f ", creal(*value));
    for (i=0; i<number+e; i++)
      fprintf(fp, "%.4f ", crealf(v[i]));
    fprintf(fp, "\n");
  }


  // In case of AC:
  // We print first the magnitude and then the phase of each quantity.

  else {
    fprintf (fp, "%.4f %.4f ", pow(crealf(*value)*crealf(*value) + cimagf(*value)*cimagf(*value), 0.5), atanf(cimagf(*value)/crealf(*value)));
    for (i=0; i<number+e; i++)
      fprintf (fp, "%.4f %.4f ", pow(crealf(v[i])*crealf(v[i])+cimagf(v[i])*cimagf(v[i]), 0.5), atanf(cimagf(v[i])/crealf(v[i])));
    fprintf(fp, "\n");
      }
}
  


// ******************************
// ***** FUNCTION : GETNODE *****
// ******************************

/* This function goes through the linked list and checks the value of every node present in n1 & n2.
   An array with large number of int elements is created. It is initialized to all zeros.
   When a node is read, the array element corresponding to  that number is increased by 1.
   When another node is read, it checks for the corresponding array element. If it is nonzero, that means the node has been read before.
   It is skipped and the fucntion goes on for the next value of node.
   When the function exits, the array is read and checked for number of distinct nodes. A for loop assigns name to all the nodes.*/
   
void getnode (Elem * head, int *nodelist) {
      
  Elem *new, *old = NULL;
  new = head;
  
  while (new != NULL) {
    
    old = new;
    new = new->next;

    if (nodelist[old->n1] == 0)
      nodelist [old->n1] ++;
    if (nodelist[old->n2] == 0)
      nodelist [old->n2] ++;
    
    old = NULL;
  }
}



// *******************************
// ***** FUNCTION : FREELIST *****
// *******************************

/* This function frees up the space allocated to the linked list.
   Input parameter is the pointer to the first element of the linked list. */

void freelist (Elem *head) {

  Elem *new, *old;
  new = head;

  while (new != NULL) {

    old = new;
    new = new->next;
    free (old->name);
    free (old->value);

    if (old->n3 != NULL && old->n4 !=NULL) {
      free (old->n3);
      free (old->n4);
    }

    if (old->depname != NULL)
      free (old->depname);
    free (old);
    old = NULL;
  }
}
    

// *****************************
// ***** FUNCTION : GETVAL *****
// *****************************

/* This function reads the "value" parameter of element written in standard notation and converts it to float number.
   Return value is 1 when the "value" is succesfully read and converted.
   Otherwise, output is 0. */

int getval (complex *result, char *str) {
  
  long int arbit = random();
  float num = (float) arbit;
  char* exp = malloc(6*sizeof(char));
  
  switch (sscanf(str, "%f%s", &num, exp)) {
    
  // When no element is read.
  case 0: return 0; break;
  
  /* When only one element is read, there are two possibilities:
     1- only float value is read
     2- only the number is read without suffix.
     Both are considered here.*/
     
  case 1: {
    if (num != (float) arbit) {
      *result = num;
      return 1;
    }
    else
      return 0;
    break;
  }
    
  /* When both elements are read. In this case we use strcmp to compare
     the two strings and return the result accordingly. */

  case 2: {
    
    if (strcmp (exp, "k") == 0 || strcmp (exp, "K") == 0) {
      *result = num * 1000;
      return 1;
    }
    
    else if (strcmp (exp, "m") == 0 || strcmp(exp, "M") == 0) {
      *result = num / 1000;
      return 1;
    }
      
    else if (strcmp (exp, "meg") == 0 || strcmp (exp, "MEG") == 0) {
      *result = num * 1000000;
      return 1;
    }
      
    else if (strcmp (exp, "u") == 0 || strcmp (exp, "U") == 0) {
      *result = num / 1000000;
      return 1;
    }
      
    else if (strcmp (exp, "n") || strcmp (exp, "N")) {
      *result = num / 1000000000;
      return 1;
    }
      
    else
      return 0;
			
    break;
  }
  
  }
}
