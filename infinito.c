#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

typedef struct {
    sigset_t msk;
    
    pid_t h1, h2, h3, h4, n1, n2, P;
    int listo;
    int vueltas;
} Glob;
Glob g;


void comprobar(int signum){
	  g.listo = 1;
}

void fin(int signum){	
    if(g.h2 != -1){
        kill(g.h2, SIGUSR2);
	      g.listo=0;
	      while(g.listo==0) {
        	  sigsuspend(&(g.msk));
        }
    }
	  
    if (g.h3 != -1){
        kill(g.h3, SIGUSR2);
	      g.listo=0;
	      while(g.listo==0) {
        	  sigsuspend(&(g.msk));
        }
    }   
	  
    if(g.h1 != -1){
        kill(g.h1, SIGKILL);
  	    wait(NULL);
    }

    if(g.h2 != -1){
        kill(g.h2, SIGKILL);
	      wait(NULL);
    }

    if(g.h3 != -1){
        kill(g.h3, SIGKILL);
	      wait(NULL);
    }

    if(g.h4 != -1){
        kill(g.h4, SIGKILL);
  	    wait(NULL);
    }
	  printf("La señal ha dado %d vueltas\n", (int) g.vueltas/2);
	  raise(SIGKILL);
}

void fin_n(int signum){
	  if(g.n1 != -1 && g.n1 != 0) {
	     	kill(g.n1, SIGKILL);
	    	wait(NULL);
    		kill(getppid(), SIGUSR1);
    		sigsuspend(&(g.msk));
  	}else if(g.n2 != -1 && g.n2 != 0) {
	    	kill(g.n2, SIGKILL);
    		wait(NULL);
    		kill(getppid(), SIGUSR1);
    		sigsuspend(&(g.msk));
  	}
}

int main(void){
        sigset_t bloq;
        sigfillset(&bloq);        
        if (-1==sigprocmask(SIG_BLOCK, &bloq, NULL)){
		        perror("ERROR");
		        return 1;
	      }

        g.P = -1;
        g.h1 = -1;
        g.h2 = -1;
        g.h3 = -1;
        g.h4 = -1;
        g.n1 = -1;
        g.n2 = -1;

        g.P = getpid();

        sigfillset(&g.msk);
	      sigdelset(&g.msk, SIGUSR1);
	      sigdelset(&g.msk, SIGUSR2);

        pid_t Hijo1, Hijo2, Hijo3, Hijo4, Nieto1, Nieto2, Padre;

        sigset_t def;
        sigfillset(&def);
        sigdelset(&def, SIGALRM);
        sigdelset(&def, SIGUSR1);

        sigset_t hij;
        sigfillset(&hij);
        sigdelset(&hij, SIGUSR2);
        sigdelset(&hij, SIGUSR1);      
	
	      alarm(25);
	
	      //MANEJADORA PARA RECIBIR SIGUSR1
	      struct sigaction ss;	
        ss.sa_handler=comprobar;
        sigemptyset(&ss.sa_mask);
        ss.sa_flags=0;
        if (-1==sigaction(SIGUSR1, &ss, NULL)){
		        perror("ERROR");
		        return 1;
	      }

        //MANEJADORA PARA MATAR A LOS NIETOS
        struct sigaction fin_nieto;
        fin_nieto.sa_handler=fin_n;
        sigemptyset(&fin_nieto.sa_mask);
        fin_nieto.sa_flags=0;
        if (-1==sigaction(SIGUSR2, &fin_nieto, NULL)){
		        perror("ERROR");
		        return 1;
	    }
        
        //MANEJADORA PARA EL FINAL DEL PROGRAMA
        struct sigaction final;
        final.sa_handler=fin;
        sigemptyset(&final.sa_mask);
        final.sa_flags=0;
        if (-1==sigaction(SIGALRM, &final, NULL)){
		        perror("ERROR");
		        return 1;
	      }
        
        
	
	      g.h1 = fork();	
        switch (g.h1){
          case -1:
            perror("ERROR");
            kill(g.P, SIGALRM);
            exit(1);
          case 0:
            Hijo1 = getpid();
            kill(getppid(),SIGUSR1);
            break;  
          default:
            while(g.listo==0) {
         	      sigsuspend(&def);
            }                    
            g.listo = 0;           
            g.h4 = fork();
            switch (g.h4) {
              case -1:
                perror("ERROR");
                kill(g.P, SIGALRM);
                exit(1);
              case 0:
                Hijo4 = getpid();
                kill(getppid(),SIGUSR1);
                break;
              default:        
                while(g.listo==0) {
         	          sigsuspend(&def);
                }                                     
                g.listo = 0;
                g.h2 = fork();
                switch (g.h2) {
                  case -1:
                    perror("ERROR");
                    exit(1);
                  case 0:    
                    Hijo2 = getpid();           
                    g.n1 = fork();
                    switch (g.n1) {
                      case -1:
                        perror("ERROR");
                        kill(g.P, SIGALRM);
                        exit(1);
                      case 0:
                        Nieto1 = getpid();
                      	kill(getppid(),SIGUSR1);
                        break;
                      default:
                      	while(g.listo==0) {
         	  	              sigsuspend(&def);
                	      }               	
                	      kill(getppid(),SIGUSR1);
                        break;
                    }
                    break;
                  default:
                    while(g.listo==0) {
         	  	          sigsuspend(&def);
                    }              	
                    g.listo = 0;                      
                    g.h3 = fork();
                    switch (g.h3) {
                      case -1:
                        perror("ERROR");
                        kill(g.P, SIGALRM);
                        exit(1);
                      case 0:   
                        Hijo3 = getpid();                  
                        g.n2 = fork();
                        switch (g.n2) {
                          case -1:
                           perror("ERROR");
                           kill(g.P, SIGALRM);
                           exit(1);
                          case 0:
                            Nieto2 = getpid();
                            kill(getppid(),SIGUSR1);
                            break;
                          default:
                            while(g.listo==0) {
         	  	  	              sigsuspend(&def);
                    	      }
         		                kill(getppid(),SIGUSR1);    	
                            break;
                        }
                        break;
                      default:
                        while(g.listo==0) {
         	  	              sigsuspend(&def);
                        }                   	
                      	g.listo = 0; 
                        Padre = getpid(); 
                   	    break;
                    }
                    
                }
              
            }
        
        } 

        if (getpid() != Padre) {
            sigsuspend(&hij);
        }


        
        int turno = 0;
        while(1) {
            if(getpid() == Padre && Padre != 0){
              while(1){
                  g.vueltas++;
        			    if (turno == 0){
        				      kill(g.h2,SIGUSR1);
                      turno = 1;
        				      sigsuspend(&def);
        		      } else if (turno == 1){
        				      kill(g.h3,SIGUSR1);
                      turno = 0;
        				      sigsuspend(&def);
        			    }  
              }  			
            }else if(getpid() == Hijo2 && Hijo2 != 0){
                  while(1){
                      kill(g.n1,SIGUSR1);
        				      sigsuspend(&hij);
                  }                  
            }else if(getpid() == Nieto1 && Nieto1 != 0){
                  while(1){
                      kill(g.h1,SIGUSR1);
        				      sigsuspend(&hij);
                  }
                  
            }else if(getpid() == Hijo1 && Hijo1 != 0){
                  while(1){
                      kill(g.P,SIGUSR1);
        				      sigsuspend(&hij);
                  }
                  
            }else if(getpid() == Hijo3 && Hijo3 != 0){
                  while(1){
                      kill(g.n2,SIGUSR1);
        				      sigsuspend(&hij);
                  }
                  
            }else if(getpid() == Nieto2 && Nieto2 != 0){
                  while(1){
                      kill(g.h4,SIGUSR1);
        				      sigsuspend(&hij);
                  }
                  
            }else if(getpid() == Hijo4 && Hijo4 != 0){
                  while(1){
                      kill(g.P,SIGUSR1);
        				      sigsuspend(&hij);
                  }
                  
            }
        }
	 
	      return 0;
}


