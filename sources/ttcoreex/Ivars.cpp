/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
#include "stdafx.h"
#include "tintin.h"
#include "ttobjects.h"

int stacks[100][3];

extern struct listnode *searchnode_list();

int do_one_inside(int begin, int end);


void math_command(wchar_t *line)
{
  wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE], 
       temp[BUFFER_SIZE], result[BUFFER_SIZE];
  int i; 
  line=get_arg_in_braces(line,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);
  line=get_arg_in_braces(line,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);
  substitute_vars(right, result, sizeof(result)/sizeof(wchar_t));
  substitute_myvars(result, right, sizeof(right)/sizeof(wchar_t));
  i=eval_expression(right);
  swprintf(temp,L"%d",i);
  

  
    VAR_INDEX ind = VarList.find(left);
    VAR* pvar;
    if ( ind != VarList.end() ) {
        pvar = ind->second;
        pvar->m_strVal = temp;
    }
    else {
        pvar = new VAR(temp);
        VarList[left] = pvar;
    }
}

void if_command(wchar_t *line)
{
  wchar_t if_expr[BUFFER_SIZE], 
	   if_then[BUFFER_SIZE], 
       if_else[BUFFER_SIZE], 
       temp[BUFFER_SIZE];

  line=get_arg_in_braces(line,if_expr,STOP_SPACES,sizeof(if_expr)/sizeof(wchar_t)-1);
      substitute_vars(if_expr,temp, sizeof(temp)/sizeof(wchar_t));substitute_myvars(temp,if_expr, sizeof(if_expr)/sizeof(wchar_t));
  line=get_arg_in_braces(line,if_then,WITH_SPACES,sizeof(if_then)/sizeof(wchar_t)-1);
      substitute_vars(if_then,temp, sizeof(temp)/sizeof(wchar_t));substitute_myvars(temp,if_then, sizeof(if_then)/sizeof(wchar_t));
  line=get_arg_in_braces(line,if_else,WITH_SPACES,sizeof(if_else)/sizeof(wchar_t)-1);
      substitute_vars(if_else,temp, sizeof(temp)/sizeof(wchar_t));substitute_myvars(temp,if_else, sizeof(if_else)/sizeof(wchar_t));


	wchar_t *to_parse = eval_expression(if_expr) ? if_then : if_else;
	if( to_parse && wcslen(to_parse) ) {
		parse_input(to_parse); 
	}
}


int eval_expression(wchar_t *arg)
{
  /* int i, begin, end, flag, prev, ptr; */
  int i, begin, end, flag, prev;
  wchar_t temp[BUFFER_SIZE];
  i=conv_to_ints(arg);
  if (i) {
    while(1) {
      i=0;
      flag=1;
      begin= -1;
      end= -1;
      prev= -1;
      while(stacks[i][0] && flag) {
        if (stacks[i][1]==0) {
          begin=i;
        }
        else if(stacks[i][1]==1) {
          end=i;
          flag=0;
        }
        prev=i;
        i=stacks[i][0];
      }
      if ((flag && (begin!= -1)) || (!flag && (begin== -1))) {
        tintin_puts2(rs::rs(1084));
        return 0;
      }
      if (flag) {
        if (prev== -1)
          return(stacks[0][2]);
        begin= -1;
        end=i;
      }
      i=do_one_inside(begin,end);
      if (!i) {
        swprintf(temp, rs::rs(1085), arg);
        tintin_puts2(temp);
        return 0;
      }
    }   
  }
  else return 0;  
}
int conv_to_ints(wchar_t *arg)
{
  int i, flag;
  wchar_t *ptr, *tptr;
  i=0;
  ptr=arg;
  while (*ptr) {
    if (*ptr==L' ') ;
    else if (*ptr==L'(') {
      stacks[i][1]=0;
    }
    else if(*ptr==L')') {
      stacks[i][1]=1;
    }
    else if(*ptr==L'!') {
      if (*(ptr+1)==L'=') { 
        stacks[i][1]=12;
        ptr++;
      }
      else
        stacks[i][1]=2;
    }
    else if(*ptr==L'*') {
      stacks[i][1]=3;
    }
    else if(*ptr==L'/') {
      stacks[i][1]=4;
    }
    else if(*ptr==L'+') {
      stacks[i][1]=5;
    }
    else if(*ptr==L'-') {
      flag= -1;
      if (i>0)
        flag=stacks[i-1][1];
      if (flag==15)
        stacks[i][1]=6;
      else {
        tptr=ptr;
        ptr++;
        while(iswdigit(*ptr))
          ptr++;
        swscanf(tptr,L"%d",&stacks[i][2]);
        stacks[i][1]=15;
        ptr--;
      }
    }
    else if(*ptr==L'>') {
      if (*(ptr+1)==L'=') {
        stacks[i][1]=8;
        ptr++;
      }
      else
        stacks[i][1]=7;
    }
    else if(*ptr==L'<') {
      if (*(ptr+1)==L'=') {
        ptr++;
        stacks[i][1]=10;
      }
      else
        stacks[i][1]=9;
    }
    else if(*ptr==L'=') {
      stacks[i][1]=11;
      if (*(ptr+1)==L'=')
        ptr++;
    }
    else if(*ptr==L'&') {
      stacks[i][1]=13;
      if (*(ptr+1)==L'&')
        ptr++;
    }
    else if(*ptr==L'|') {
      stacks[i][1]=14;
      if (*(ptr+1)==L'|')
        ptr++;
    }
    else if (iswdigit(*ptr)) {
      stacks[i][1]=15;
      tptr=ptr;
      while (iswdigit(*ptr))
        ptr++;
      swscanf(tptr,L"%d",&stacks[i][2]);
      ptr--;
    }
    else if (*ptr==L'T') {
      stacks[i][1]=15;
      stacks[i][2]=1;
    }
    else if(*ptr==L'F') {
      stacks[i][1]=15;
      stacks[i][2]=0;
    }
    else {
      tintin_puts2(rs::rs(1086));
      return 0;
    }
    if (*ptr!=L' ') {
      stacks[i][0]=i+1;
      i++;
    }
    ptr++;
  }
  if (i>0)
    stacks[i][0]=0;
  return 1;
}

int do_one_inside(int begin, int end)
{
  /* int prev, ptr, highest, loc, ploc, next, nval, flag; */
  int prev, ptr, highest, loc, ploc, next;
  while(1) {
    ptr=0;
    if (begin>-1)
      ptr=stacks[begin][0];
    highest=16;
    loc= -1;
    ploc= -1;
    prev= -1;
    while (ptr<end) {
      if (stacks[ptr][1]<highest) {
        highest=stacks[ptr][1];
        loc=ptr;
        ploc=prev;
      }
      prev=ptr;
      ptr=stacks[ptr][0];
    }
    if (highest==15) {
      if (begin>-1) {
        stacks[begin][1]=15;
        stacks[begin][2]=stacks[loc][2];
        stacks[begin][0]=stacks[end][0];
        return 1;
      }
      else {
        stacks[0][0]=stacks[end][0];
        stacks[0][1]=15;
        stacks[0][2]=stacks[loc][2];
        return 1;
      }
    }
    else if (highest==2) {
      next=stacks[loc][0];
      if (stacks[next][1]!=15 || stacks[next][0]==0) {
        return 0;
      }  
      stacks[loc][0]=stacks[next][0];
      stacks[loc][1]=15;
      stacks[loc][2]=!stacks[next][2];
    }  
    else {
      next=stacks[loc][0];
      if (ploc== -1 || stacks[next][0]==0 || stacks[next][1]!=15) 
        return 0;
      if (stacks[ploc][1]!=15)
        return 0;    
      switch (highest) {
        case 3: /* highest priority is * */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]*=stacks[next][2];
          break;
        case 4: /* highest priority is / */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]/=stacks[next][2];
          break;
        case 5: /* highest priority is + */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]+=stacks[next][2];
          break;
        case 6: /* highest priority is - */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]-=stacks[next][2];
          break;
        case 7: /* highest priority is > */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]=(stacks[ploc][2]>stacks[next][2]);
          break;
        case 8: /* highest priority is >= */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]=(stacks[ploc][2]>=stacks[next][2]);
          break;
        case 9: /* highest priority is < */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]=(stacks[ploc][2]<stacks[next][2]);
          break;
        case 10: /* highest priority is <= */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]=(stacks[ploc][2]<=stacks[next][2]);
          break;
        case 11: /* highest priority is == */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]=(stacks[ploc][2]==stacks[next][2]);
          break;
        case 12: /* highest priority is != */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]=(stacks[ploc][2]!=stacks[next][2]);
          break;
        case 13: /* highest priority is && */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]=(stacks[ploc][2]&&stacks[next][2]);
          break;
        case 14: /* highest priority is || */
          stacks[ploc][0]=stacks[next][0];
          stacks[ploc][2]=(stacks[ploc][2]||stacks[next][2]);
          break;
        default:
          tintin_puts2(rs::rs(1087));
          return 0;     
      }
    }
  }
}

void strcmp_command(wchar_t *arg) 
{
	wchar_t arg1[BUFFER_SIZE], 
		 arg2[BUFFER_SIZE], 
		 if_then[BUFFER_SIZE], 
		 if_else[BUFFER_SIZE],
		 temp[BUFFER_SIZE];

	arg = get_arg_in_braces(arg,arg1,STOP_SPACES,sizeof(arg1)/sizeof(wchar_t)-1);
	substitute_vars(arg1,temp, sizeof(temp)/sizeof(wchar_t));
	substitute_myvars(temp,arg1, sizeof(arg1)/sizeof(wchar_t));

	arg = get_arg_in_braces(arg,arg2,STOP_SPACES,sizeof(arg2)/sizeof(wchar_t)-1);
	substitute_vars(arg2,temp, sizeof(temp)/sizeof(wchar_t));
	substitute_myvars(temp,arg2, sizeof(arg2)/sizeof(wchar_t));
 
	arg = get_arg_in_braces(arg,if_then,WITH_SPACES,sizeof(if_then)/sizeof(wchar_t)-1);
    substitute_vars(if_then,temp, sizeof(temp)/sizeof(wchar_t));
	substitute_myvars(temp,if_then, sizeof(if_then)/sizeof(wchar_t));

	arg = get_arg_in_braces(arg,if_else,WITH_SPACES,sizeof(if_else)/sizeof(wchar_t)-1);
    substitute_vars(if_else,temp, sizeof(temp)/sizeof(wchar_t));
	substitute_myvars(temp,if_else, sizeof(if_else)/sizeof(wchar_t));

	wchar_t *to_parse = !wcscmp(arg1, arg2) ? if_then : if_else;
	if( to_parse && wcslen(to_parse) ) {
		parse_input(to_parse); 
	}
}

void match_command(wchar_t *arg) 
{
	wchar_t pattern[BUFFER_SIZE], 
		 strng[BUFFER_SIZE], 
		 if_then[BUFFER_SIZE], 
		 if_else[BUFFER_SIZE],
		 temp[BUFFER_SIZE];
	
	CPCRE re;

	arg = get_arg_in_braces(arg,temp,STOP_SPACES,sizeof(temp)/sizeof(wchar_t)-1);
	prepare_actionalias(temp, pattern, sizeof(pattern)/sizeof(wchar_t)-1);

    BOOL i_flag = FALSE, m_flag = FALSE, g_flag = FALSE;
	std::wstring regexp = pattern;

    if ( *pattern == L'/' ) {
		regexp = (wchar_t*)pattern + 1;
        
        int size = regexp.size();
		
		for (int i = size - 1; i >= 0; i--) {
			if (regexp[i] == L'i') {
				size--;
				i_flag = TRUE;
			} else if (regexp[i] == L'm') {
				size--;
				m_flag = TRUE;
			} else if (regexp[i] == L'g') {
				size--;
				g_flag = TRUE;
			} else if (regexp[i] == L'/') {
				size--;
				break;
			} else {
				size = regexp.size();
				i_flag = m_flag = g_flag = FALSE;
				break;
			}
		}
		regexp.resize(size);
    }

	if (!re.SetSource(regexp, m_flag, i_flag)) {
		return;
	}

	arg = get_arg_in_braces(arg,temp,STOP_SPACES,sizeof(temp)/sizeof(wchar_t)-1);
	prepare_actionalias(temp, strng, sizeof(strng)/sizeof(wchar_t)-1);
 
	arg = get_arg_in_braces(arg,if_then,WITH_SPACES,sizeof(if_then)/sizeof(wchar_t)-1);

	arg = get_arg_in_braces(arg,if_else,WITH_SPACES,sizeof(if_else)/sizeof(wchar_t)-1);

	int offset = 0;
    int offsets[33];

	bool no_match = true;
	wchar_t *test = (wchar_t*)strng;
	int test_len = wcslen(test);

	for (;;) {
		int captured = pcre16_exec(re.m_pPcre, re.m_pExtra, test, test_len, offset, 0, offsets, 33);
		if (captured <= 0)
			break;

		no_match = false;
		
		int i;
		for (i = 0; i < 10; i++) 
			vars[i][0] = 0;
		for (i = 1; i < captured; i++) {
			if (offsets[i*2] >= 0) {
				int size = offsets[i*2 + 1] - offsets[i*2];
				wcsncpy(vars[i-1], test + offsets[i*2], size);
				*(vars[i-1]+size) = L'\0'; 
			}
		}
		prepare_actionalias(if_then, temp, sizeof(temp)/sizeof(wchar_t)-1);
		parse_input(temp, g_flag);

		offset = offsets[1];

		if (!g_flag)
			break;
	}

	if (no_match)
		parse_input(if_else);
}
