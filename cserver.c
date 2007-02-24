/******************************************************************************
* Id: $Id$                      *
*                                                                             *
* Alberto Montañola Lacort                                                    *
*                                                                             *
* Xarxes II                                                                   *
* Màster en Enginyeria de Programari Lliure                                   *
*                                                                             *
*                                                                             *
******************************************************************************/

//Data types
//If we have an architecture whose data type does not match the corresponding
// number of bytes, we must change it here.
typedef unsigned char Byte;
typedef unsigned short int U16;
typedef unsigned int U32;

int __state_running=1; //Is the server running?

/// Show server usage information
/// @param pgname Program name
void usage(char * pgname) {
	printf("Usage: %s [-p 8000]\n\
 -p <port>: Select the listening port\n\
\n");
}

/// Server Signal Handler
/// @param s Signal number
void s_handler(int s) {

}

int main(int argc, char * argv) {


}
