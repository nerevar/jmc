/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: path.c - stuff for the path feature                         */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                    coded by peter unold 1992                      */
/*                  recoded by Jeremy C. Jack 1994                   */
/*********************************************************************/
/* the path is implemented as a fix-sized queue. It gets a bit messy */
/* here and there, but it should work....                            */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"


void check_insert_path();
void insert_path();
int return_flag=TRUE;
//* en:race
char race_format[BUFFER_SIZE];
char race_last[BUFFER_SIZE];
//*/en

extern char *get_arg_in_braces();
extern struct listnode *search_node_with_wild();
extern struct listnode *searchnode_list();
extern struct listnode *init_list();


struct listnode *init_pathdir_list(void)
{
  struct listnode *listhead;

  if((listhead=(struct listnode *)(malloc(sizeof(struct listnode))))==NULL) {
    return NULL;
  }
  listhead->next=NULL;

    insertnode_list(listhead, "n", "s", "0", ALPHA);
    insertnode_list(listhead, "s", "n", "0", ALPHA);
    insertnode_list(listhead, "w", "e", "0", ALPHA);
    insertnode_list(listhead, "e", "w", "0", ALPHA);
    insertnode_list(listhead, "d", "u", "0", ALPHA);
    insertnode_list(listhead, "u", "d", "0", ALPHA);

  return(listhead);
}


void mark_command(char* arg)
{
    if ( !arg || !*arg  || !strcmp("start", arg) ) {
        kill_list(common_path);
        common_path=init_list();
        path_length=0;
        bPathing = TRUE;
        tintin_puts2(rs::rs(1147));
    }else {
        bPathing = FALSE;
        tintin_puts2(rs::rs(1148));
    }

}

void map_command(char *arg)
{
    if ( bPathing ) {
        get_arg_in_braces(arg, arg, WITH_SPACES);
        check_insert_path(arg);
    } else {
        tintin_puts2(rs::rs(1149));
    }
}

void savepath_command(char* arg)
{
    char result[BUFFER_SIZE];
    struct listnode *ln=common_path;
    int dirlen, len=0;
    char left[BUFFER_SIZE], right[BUFFER_SIZE];

    if ( !path_length ) {
        tintin_puts2(rs::rs(1150));
        return;
    }

    arg = get_arg_in_braces(arg, left, STOP_SPACES);
    arg = get_arg_in_braces(arg, right, WITH_SPACES);

    if ( !left[0] /*|| (right[0] && strcmp(right , "revers"))*/ ) {
        tintin_puts2(rs::rs(1151));
        return;
    }


    sprintf(result, "%calias {%s} {", cCommandChar, left);
    len = strlen(result);

    if ( !right[0] ) {
        while (ln=ln->next) {
            dirlen = strlen(ln->left);
            if (dirlen+len+2<BUFFER_SIZE-1) {
                strcat(result, ln->left);
	            len+=dirlen+1;
                if (ln->next) {
                    result[len-1] = cCommandDelimiter;
                    result[len] = 0;
                }
            } else {
	            tintin_puts2(rs::rs(1152));
	            return;
            }
        }
    }
    else {
        // Do backscroll. I have to make stack for nodes
        struct listnode** nodes = (struct listnode**)malloc(sizeof(struct listnode*)*(path_length+1));
        int i = 0;
        while ( ln=ln->next) 
            nodes[i++] = ln;
        // i = path_length-1;
        while (i > 0 ) {
            dirlen = strlen(nodes[i-1]->right);
            if (dirlen+len+2<BUFFER_SIZE-1) {
                strcat(result, nodes[i-1]->right);
	            len+=dirlen+1;
                if (i) {
                    result[len-1] = cCommandDelimiter;
                    result[len] = 0;
                }
            } else {
	            tintin_puts2(rs::rs(1153));
	            return;
            }
            i--;
        }
    }
    
    strcat(result, "}");
    parse_input(result);
}

void path_command(char*arg)
{
    int len=0, dirlen;
    struct listnode *ln=common_path;
    char mypath[81];
    strcpy(mypath, rs::rs(1154));
    while (ln=ln->next) {
      dirlen = strlen(ln->left);
      if (dirlen+len>70) {
	    tintin_puts2(mypath);
        strcpy(mypath, rs::rs(1154));
        len=0;
      } 
      strcat(mypath, ln->left);
      strcat(mypath, " ");
      len += dirlen+1;
    }
    tintin_puts2(mypath);
}

void return_command(char*arg)
{
    if (path_length) {
      struct listnode *ln=common_path;
      char command[BUFFER_SIZE];
      path_length--;
      while (ln->next) (ln=ln->next);
      strcpy(command, ln->right);
      return_flag=FALSE;	/* temporarily turn off path tracking */
      parse_input(command);
      return_flag=TRUE;		/* restore path tracking */
      deletenode_list(common_path, ln);
    } else tintin_puts2(rs::rs(1156));
}

void unpath_command(char*arg)
{
    if (path_length) {
      struct listnode *ln=common_path;
      path_length--;
      while (ln->next) (ln=ln->next);
      deletenode_list(common_path, ln);
      tintin_puts2(rs::rs(1157));
    } else tintin_puts2(rs::rs(1158));
}

void check_insert_path(char *command)
{
  struct listnode *ln;

  if (!return_flag || !bPathing)
    return;

  if((ln = searchnode_list(common_pathdirs, command))!=NULL) {
    if (path_length!=MAX_PATH_LENGTH) path_length++;
      else if (path_length)
	deletenode_list(common_path, common_path->next);
    addnode_list(common_path, ln->left, ln->right, "0");
  }
}

void pathdir_command(char *arg)
{
  char left[BUFFER_SIZE], right[BUFFER_SIZE], arg2[BUFFER_SIZE];
  struct listnode *mypathdirs, *ln;

  mypathdirs=common_pathdirs;
  arg = get_arg_in_braces(arg, left, STOP_SPACES);
  arg = get_arg_in_braces(arg, right, WITH_SPACES);

  if (!*left) {
    tintin_puts2(rs::rs(1159));
    show_list(mypathdirs);
  } else if(*left&&!*right) {
    if ((ln=search_node_with_wild(mypathdirs, left))!=NULL) {
      while ((mypathdirs=search_node_with_wild(mypathdirs, left))!=NULL)
        shownode_list(mypathdirs);
    } else 
        tintin_puts2(rs::rs(1160));
  } else {
    if ((ln=searchnode_list(mypathdirs, left))!=NULL) {
        free(ln->right);
        ln->right = (char *)malloc(strlen(right)+1);
        strcpy(ln->right, right);
    }
    else {
        insertnode_list(mypathdirs, left, right, "9", PRIORITY);
    }
    sprintf(arg2, rs::rs(1161),left, right);
    tintin_puts2(arg2);
    }
}

//* en:race
void race_command(char *arg)
{
	char mode[BUFFER_SIZE],
  	  command[BUFFER_SIZE],
	  *cptr1,*cptr2,*cptr3;
	if(!race_format[0])
	  strcpy(race_format,"@");
	if(!race_last[0])
	  strcpy(race_last,"");

	arg = get_arg_in_braces(arg,mode, STOP_SPACES);
	arg = get_arg_in_braces(arg,command, WITH_SPACES);

	if(mode[0]=='f' && is_abrev(mode, "format"))
	{
		  if ( !command || !*command ) {
	  	  	  strcpy(command,"%0\0");
  	  	  }
  	  	  BOOL hasD = FALSE;
  	  	  for(cptr1=command;*cptr1&&*cptr1!='\0';cptr1++)
    	  	  if(*cptr1=='%'&&*(cptr1+1)=='0')
	  	  	  	  hasD = TRUE;
  	  	  if(!hasD) 
	  	  	  strcpy(cptr1," %0\0");
		  strcpy(race_format,command);

	}
	else
	if(mode[0]=='m' && is_abrev(mode, "move"))
	{
		  if ( command && *command ) {
			cptr1=command;
			cptr3=race_last;
			for(;*cptr1&&*cptr1!='\0';)
				*cptr3++=*cptr1++;
			*cptr3='\0';
  	  	  }
   	  	  char cbuffer[BUFFER_SIZE];

            sprintf(vars[0], "%s", race_last);
            substitute_myvars(race_format,cbuffer);
		 parse_input(cbuffer);
	}
	else
	if(mode[0]=='r' && is_abrev(mode, "reverse"))
	{
	  	char cbuffer[BUFFER_SIZE],
			 dbuffer[BUFFER_SIZE],
	  	      result[BUFFER_SIZE];
		char *strt=cbuffer;
		result[0]='\0';
		for(int i = 0;race_last[i];i++)
		{
          *strt=race_last[i];
          if(*strt=='n'||*strt=='e'||*strt=='s'||*strt=='w'||*strt=='d'||*strt=='u')
		  {
           switch(*strt)
		   {
            case 'n':*strt='s';break;
            case 's':*strt='n';break;
            case 'd':*strt='u';break;
            case 'u':*strt='d';break;
            case 'w':*strt='e';break;
            case 'e':*strt='w';break;
		   }
		   *(strt+1)='\0';
		   cptr1=result;
		   cptr3=dbuffer;
		   for(;*cptr1&&*cptr1!='\0';)
			   *cptr3++=*cptr1++;
		   *cptr3='\0';
           cptr1=result;
		   cptr2=cbuffer;
		   cptr3=dbuffer;
		   for(;*cptr2&&*cptr2!='\0';)
			   *cptr1++=*cptr2++;
		   for(;*cptr3&&*cptr3!='\0';)
			   *cptr1++=*cptr3++;
		   *cptr1='\0';
		   strt=cbuffer;
		  }
		  else
			  strt++;
		}
	   cptr1=result;
	   cptr3=race_last;
	   for(;*cptr1&&*cptr1!='\0';)
		   *cptr3++=*cptr1++;
	   *cptr3='\0';

	   race_command("move");
	}
}
//*/en
