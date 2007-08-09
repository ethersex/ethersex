/*
 * tetrinet authentication string cracker
 * taken from tetrinetx and slightly adjusted to fit tetrirape needs
 */


#include <stdio.h>
#include <string.h>

#define lvprintf(a...) ((void) 0)

/*
  crack.c

  This is the initial encryption-decryption module
*/

static unsigned char tet_decryptchar(int enctype, unsigned char prev, unsigned char cur);

/* tet_dec2str(decrypted hex init string) */
/*   Converts each hex value from the decrypted hex init string into */
/*   corrosponding characters. */
void tet_dec2str(char *buf)
  { /* Takes a decrypted string, and returns a string from the hex values */
    //char *strg;
    char *P, *Q;
    unsigned int curr;
    
    //strg = malloc((strlen(buf)/2)+4);
    
    P=buf+26;
    Q=buf;
    
    //P+=2; /* Skip the salt */
    
    while( *P != '\0' )
      {
        sscanf(P,"%2X", &curr);
        if((*Q=curr) == ' ') break;
        P+=2;
        Q++;
      }
      
    *Q=0;
    buf[15] = 0;  				/* crop nickname to 15 chars */

    //return(strg);
  }

/* tet_decrypt(encrypted init string)                                */
/*   Scans string, tries to learn the encryption type and key from the */
/*   first couple of values, then uses this on the rest. Overwrites */
/*   encrypted string with decrypted hex string */
/*     This was done because it means the encryption is IP independent!!!! */
int tet_decrypt(char *buf)
  { /* Decrypts an entire string. Overwrites encrypted string */
    /* Returns 0 on success. -1 if failed */
    
    char done;
    char *P;
    char back[3];
    int j,k=0;
    unsigned int prev;
    unsigned int curr;
    unsigned char curr_dec;
    unsigned int enctype[10]; 	/* The order of the decrypt sequences */
    const unsigned int decrypt_data[10] = /* tetrisstart */
/*        {0xB4, 0xA5, 0xB4, 0xB2, 0xA9, 0xB3, 0xB3, 0xB4, 0xA1, 0xB2};*/
          {0x74, 0x65, 0x74, 0x72, 0x69, 0x73, 0x73, 0x74, 0x61, 0x72};

    int enctype_index, enctype_tot;
    
    enctype_index = 0;
    
    /* First, we teach ourself how to decrypt this beast */
    P = buf;
    sscanf(P,"%2X", &curr);     /* First hex pair is the salt */
    P+=2;

  lvprintf(10,"LEARN\n");
    enctype_index = 0;
    while(enctype_index < 10)
      { /* Learn from first 10 numbers */
        done=0;
        j=1;
        prev=curr;
        sscanf(P,"%2X", &curr);
        while ( (!done) && (j<20) )
          {
            curr_dec = tet_decryptchar(j,prev,curr);
            k=j;
            switch (j)
              {
                case 1: {j=2;break;}
                case 2: {j=3;break;}
                case 3: {j=4;break;}
                case 4: {j=5;break;}
                case 5: {j=6;break;}
                case 6: {j=7;break;}
                case 7: {j=8;break;}
                case 8: {j=9;break;}
                case 9: {j=10;break;}
                case 10: {j=99;break;}
                
              }
            done=( curr_dec==decrypt_data[enctype_index]);
          }
        if (!done)
          {
            lvprintf(4,"Unknown encryption type at pos %d.\n", enctype_index);
            return(-1);
          }
  lvprintf(10,"%d\n",k);
        enctype[enctype_index] = k;
        enctype_index++;
        P+=2;
      }

    /* Now see if we have a pattern in our 10 checks, and if so, reduce enctype_tot to that pattern */
    j=0;
    done=0;
    enctype_tot=10;
    while(!done && (j < 5))
      {
        j++;
        done=1;
        for (enctype_index=0; enctype_index < (10-j); enctype_index++)
          {
            done = done && (enctype[enctype_index] == enctype[enctype_index+j]);
          }
      }

    if (done)
      { /* done - Means we found a pattern ever j times */
        enctype_tot = j; 
      }


      lvprintf(10,"total=%d\n",enctype_tot);

    /* And volia, an IP independent encryption pattern :) I still don't know why it works ;)*/

    /* Survived to here, so we should have the decryption sequence */
    P = buf; /* Reset */
    enctype_index=0;
    
    sscanf(P,"%2X", &curr); 	/* First hex pair is the salt */
    P+=2;
    done = (*P == '\0');
    
    while (!done)
      {
        prev=curr;
        sscanf(P,"%2X", &curr);
        curr_dec = tet_decryptchar(enctype[enctype_index],prev,curr);
        enctype_index++; if (enctype_index >= enctype_tot) enctype_index-=(enctype_tot);
        sprintf(back,"%02X",curr_dec);
        *P    = back[0];
        *(P+1)= back[1];
        
        P+=2;
        done = (*P == '\0');
      }
    
    return(0); 
  }


/* tet_decryptchar(Type of encryption, previous encrypted hexpair, current encrypted hex pair) */
/*   Uses my standard decryption sequence on the encrypted hex pair, normalises it */
/*   and subtracts from it the last encrypted pair. */
/*   NOTE: These are the 10 "normalised" encryption techniques from an XOR */
static unsigned char tet_decryptchar(int enctype, unsigned char prev, unsigned char cur)
  {
    long int curr;
    
    curr = cur;  
  
    switch (enctype)
      { /* prev: Pp.  cur: Cc */
        case 1:  /* Type #1 */
          {
            /* - If   <0,1,8,9>
                  + Inc 6
                 else <2,3,A,B>
                  + Inc 2
                 else <4,5,C,D>
                  + Dec 2
                 else <6,7,E,F>
                  + Dec 6

               - F-c
               - FF-Cc               
            */
            if      ( (((0xf&curr)) == 0) || (((0xf&curr)) == 1) || (((0xf&curr)) == 8) || (((0xf&curr)) == 9) )
              curr+=6;
            else if ( (((0xf&curr)) == 2) || (((0xf&curr)) == 3) || (((0xf&curr)) == 0xA) || (((0xf&curr)) == 0xB) )
              curr+=2;
            else if ( (((0xf&curr)) == 4) || (((0xf&curr)) == 5) || (((0xf&curr)) == 0xC) || (((0xf&curr)) == 0xD) )
              curr-=2;
            else 
              curr-=6;
              
            curr = (unsigned char)(0xf0&curr)+(0xf-(0xf&curr));
            curr = (unsigned char)(0xff-curr);                  
            break;
          }
        
        case 4:  /* Type #4 */
          {
            /* - If   mod 4 == 0
                  + Inc 3
                 else mod 4 == 1
                  + Inc 1
                 else mod 4 == 2
                  + Dec 1
                 else mod 4 == 3
                  + Dec 3
                  
               - F-c
               - FF-Cc
            */
            if      ( (curr % 4) == 0 )
              curr+=3;
            else if ( (curr % 4) == 1 )
              curr+=1;
            else if ( (curr % 4) == 2 )
              curr-=1;
            else if ( (curr % 4) == 3 )
              curr-=3;
            
            curr = (unsigned char)(0xf0&curr)+(0xf-(0xf&curr));
            curr = (unsigned char)(0xff-curr);
            break;
          }
        case 2:  /* Type #2 */
          {
            /* - If   <even>
                  + Inc 1
                 else <odd>
                  + Dec 1
                 
               - F-c
               - FF-Cc
            */
            if ( (curr % 2) == 0 ) /* If Even */
              curr++;
            else                    /* If Odd */
              curr--;
              
            curr = (unsigned char)(0xf0&curr)+(0xf-(0xf&curr));
            curr = (unsigned char)(0xff-curr);
            break;
          }
        case 7: /* Type #7 */
          {
            /* - If   <mod 4 == 0|1>
                  + inc 2
                 else <mod 4 == 2|3>
                  + dec 2
                  
               - F-c
               - FF-Cc
            */
            if ( (curr % 4) <= 1 ) /* 0 or 1 */
              curr+=2;
            else                    /* 2 or 3 */
              curr-=2;
              
            curr = (unsigned char)(0xf0&curr)+(0xf-(0xf&curr));
            curr = (unsigned char)(0xff-curr);
            
            
            break;
          }
          
        case 5: /* Type #5 */
          {
            /* - If   <0-7>
                  + inc 8
                 else <8-F>
                  + dec 8
                  
               - If   <odd>
                  + dec 1
                 else <even>
                  + inc 1
                  
               - F-c
               - FF-Cc
            */
            if ( ((unsigned char)(0xf&curr) <= 7) )
              curr+=8;
            else
              curr-=8;
              
            if ( (curr % 2) == 0 ) /* If Even */
              curr++;
            else                    /* If Odd */
              curr--;
            curr = (unsigned char)(0xf0&curr)+(0xf-(0xf&curr));
            curr = (unsigned char)(0xff-curr);
            break;
          }
        case 6: /* Type #6 */
          {
            /* - If   <0,1,2,3,8,9,A,B>
                  + inc 4
                 else <4,5,6,7,C,D,E,F>
                  + dec 4
                  
               - If   <odd>
                  + dec 1
                 else <even>
                  + inc 1
                  
               - F-c
               - FF-Cc
            */
            if ( ( ( ((0xf&curr)) >=0x0 ) && ( ((0xf&curr)) <= 0x3 ) )
              || ( ( ((0xf&curr)) >=0x8 ) && ( ((0xf&curr)) <= 0xB ) ) )
              curr+=4;
            else
              curr-=4;
              
            if ( (curr % 2) == 0 ) /* If Even */
              curr++;
            else                    /* If Odd */
              curr--;
            curr = (unsigned char)(0xf0&curr)+(0xf-(0xf&curr));
            curr = (unsigned char)(0xff-curr);
            break;
          }
        case 10: /* Type #10 */
          {
            /* - If   <0,1,2,3,8,9,A,B>
                  + inc 4
                 else <4,5,6,7,C,D,E,F>
                  + dec 4
                  
               - F-c
               - FF-Cc
            */
            if ( ( ( ((0xf&curr)) >=0x0 ) && ( ((0xf&curr)) <= 0x3 ) )
              || ( ( ((0xf&curr)) >=0x8 ) && ( ((0xf&curr)) <= 0xB ) ) )
              curr+=4;
            else
              curr-=4;
              

            curr = (unsigned char)(0xf0&curr)+(0xf-(0xf&curr));
            curr = (unsigned char)(0xff-curr);
            break;
          }
        case 8: /* Type #8 */
          {
            /* - If   <0-7>
                  + Inc 8
                 else <8-F>
                  + Dec 8
                  
               - FF-Cc
            */
            if ( ((unsigned char)(0xf&curr) <= 7) )
              curr+=8;
            else
              curr-=8;
              
            curr = (unsigned char)(0xff-curr);
            break;
          }
        case 9: /* Type #9 */
          {
            /* - F-c
               - FF-Cc
            */
            
            curr = (unsigned char)(0xf0&curr)+(0xf-(0xf&curr));
            curr = (unsigned char)(0xff-curr);
            break;
          }
        case 3: /* Type #3 */
          {
            /* - If   <0-7>
                  + Inc 8
                 else <8-F>
                  + Dec 8
               
               - F-c   
               - FF-Cc
            */
            if ( ((unsigned char)(0xf&curr) <= 7) )
              curr+=8;
            else
              curr-=8;
              
            curr = (unsigned char)(0xf0&curr)+(0xf-(0xf&curr));
            curr = (unsigned char)(0xff-curr); break;
          }
        default:
          {
            lvprintf(2,"Internal Error - Unknown encryption type #%d\n", enctype);
            //exit(1);
	    for(;;);
            break;
          }
      }

    /* If curr is in the range:  (re-normalisation)
        - 00-3F:    inc 80+80
        - 40-7F:    inc 80
        - 80-BF:    nothing
        - C0-FF:    inc 7F
    */
    
    if      ( (curr >= 0x0) && (curr <= 0x3F) )
      curr+=(0x80+0x80);
    else if  ( (curr >= 0x40) && (curr <= 0x7F) )
      curr+=(0x80);
    else if  ( (curr >= 0xC0) && (curr <= 0xFF) )
      curr+=(0x7F); 
    
    /* Now subtract from curr, the previous value */
    curr -= prev;
    
    /* And subtract 0x40, though I have no idea why */
    curr -= 0x40;

    /* Now get it into a reasonable value */
    while (curr > 0xFF) curr -= 0xff;
    while (curr < 0x0) curr += 0xff; 
    return(curr); 
  }



#if 0
int
main(void)
{
  char test[] = "00459D27AD23A72C95C20144538CCE0A4D8693F02B6D97";
  if(tet_decrypt(test)) {
    fprintf(stderr, "failed.\n");
    return 1;
  }

  //char *result = tet_dec2str(test);
  tet_dec2str(test);
  printf("got: %s\n", test);
    
}
#endif
