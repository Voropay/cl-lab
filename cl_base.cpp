//cl_base.cpp

/*
   Copyright (C) 2013  AIDEUS
   authors: Sergey Rodionov (astroseger@gmail.com)
            Alexey Potapov  (potapov@aideus.com)
   aideus.com

   This file is part of cl-lab.

   cl-lab is released under the GNU General Public License (GNU GPL).
   Please read the included file COPYING for more information.
*/

#include "cl_base.h"
#include <algorithm>
#include <locale>
#include <sstream>
#include <stack>

cl_term::cl_term(char term_, int* counter_):term(term_)
{
   install_counter(counter_);
}
//                                                                           
/*cl_term::cl_term(const cl_term& cl)
{
   term    = cl.term;
   install_counter(cl.counter);
   typedef pair<cl_term*, const cl_term*> pair_type;
   stack<pair_type> chain_copy_stack; //to,from
   chain_copy_stack.push(pair_type(this, &cl));
   while(!chain_copy_stack.empty())
     {
	cl_term* to         = chain_copy_stack.top().first;
	const cl_term* from = chain_copy_stack.top().second;
	chain_copy_stack.pop();
	for (list<cl_term*>::const_iterator it = from->chain.begin(); it != from->chain.end() ; it++)
	  {
	     cl_term* t = new cl_term((*it)->term, (*it)->counter);
	     chain_copy_stack.push(pair_type(t, *it));
	     to->chain.push_back(t);
	  }
     }
}*/
//                                                                         
cl_term::cl_term(const cl_term& cl)
{
   term    = cl.term;
   install_counter(cl.counter);
   for (list<cl_term*>::const_iterator it = cl.chain.begin(); it != cl.chain.end() ; it++)
     {
	chain.push_back((new cl_term(*(*it))));
     }
}
//                                                                        
void cl_term::install_counter(int* counter_)
{
//   if (counter_ == NULL)
//     counter = &cl_term_global_counter;
//   else
   counter = counter_;
   (*counter)++;
}
//                                                                        
cl_term::cl_term(string str, int* counter_)
{
   term = 0;
   install_counter(counter_);   
   str = "(" + str + ")";
   //c++ is shit
   str.erase(std::remove_if(str.begin(), str.end(), (int(*)(int))isspace), str.end());
   size_t pos = 0;
   rec_create_term(str, pos);
}
//                                                                        
cl_term::~cl_term()
{
   (*counter)--;
   for (list<cl_term*>::iterator it = chain.begin(); it != chain.end() ; it++)
     delete *it;
}
//                                                                        
void cl_term::rec_print(ostream& out)
{
   out<<term;
   for (list<cl_term*>::iterator it = chain.begin(); it != chain.end() ; it++)
     {
	if ((*it)->chain.size() == 0)
	  out<<(*it)->term;
	else
	  {
	     out<<'(';
	     (*it)->rec_print(out);
	     out<<')';
	  }
     }
}
//                                                                        
/*void cl_term::non_rec_print(ostream& out)
{
   out<<term;
   typedef pair_type pair<list<cl_term*>::const_iterator, cl_term*>; 
   stack<pair_type> rps; //return point stack
   rps.push(pair_type(this,chain.begin()));
   while(!rps.empty())
     {
	list<cl_term*>::const_iterator it = rps.top().first;
	cl_term*                       t  = rps.top().second;
	bool is_stop = false;
	while(it != t.end() && !is_stop)
	  {
	     if ((*it)->chain.size() == 0)
	       out<<(*it)->term;
	     else
	       {
		  is_stop = true;
		  out<<'(';
		  rps.push_back(*it, it->chain.begin()))
		  (*it)->rec_print(out);
		  out<<')';
	       }
	     it++;
	  }
	rps.pop();
     }
   for (list<cl_term*>::iterator it = chain.begin(); it != chain.end() ; it++)
     {
	if ((*it)->chain.size() == 0)
	  out<<(*it)->term;
	else
	  {
	     out<<'(';
	     (*it)->rec_print(out);
	     out<<')';
	  }
     }
}*/
//                                                                        
string cl_term::conv2str()
{
   ostringstream out;
   rec_print(out);
   return out.str();
}
//                                                                        
bool cl_term::reduce()
{
//   rec_print(cout);
//   cout<<endl;
//   cout<<endl;
   if (term == 'S' && chain.size() >= 3)
     {
	apply_S();
	return true;       
     }
   else if (term == 'K' && chain.size() >= 2)
     {
	apply_K();
	return true;
     }
   else if (term == 'I' && chain.size() >= 1)
     {
	apply_I();
	return true;
     }
   else if (term == 'B' && chain.size() >= 3)
     {
	apply_B();
	return true;
     }
   else if (term == 'C' && chain.size() >= 3)
     {
	apply_C();
	return true;
     }
   else if (term == 'W' && chain.size() >= 2)
     {
	apply_W();
	return true;
     }
   else if (term == 'Y' && chain.size() >= 1)
     {
	apply_Y();
	return true;
     }
   return false;
}
//                                                                             
int cl_term::reduce_all(int max_steps, int max_mem, cl_resultator* resultator)
{
   int steps_left = max_steps;
   return reduce_all_(steps_left, max_mem, resultator);
}
//                                                                             
int cl_term::reduce_all_(int& steps_left, int max_mem, cl_resultator* resultator)
{
   while (reduce())
     {
	steps_left--;
	if (steps_left <= 0)
	  return -2; //hit steps limit
	if (max_mem <= *counter) 
	  return -3; //hit memory limit
     }
   //so head can be added to resultator
   if (resultator != NULL)
     {
	int rez = resultator->put_next(term);
	if (rez != 0) 
	  return rez; //-1 --- we should stop --- resultat is negative
	              //1  --- we should stop --- we obtain required resultat
     }
   for (list<cl_term*>::const_iterator it = chain.begin(); it != chain.end() ; it++)
     {
	int rez = (*it)->reduce_all_(steps_left, max_mem, resultator);
	if (rez != 0)     
	  return rez;
     }
   return 0; //so we finish but result wasn't obtained (or we want full reduce with resultator = NULL)
}
//                                                                             
void cl_term::apply_S()
{
   if (term != 'S' && chain.size() < 3)
     {
	cerr<<"Error | cl_term::apply_S | bad term application"<<endl;
     }
   //S a b c  ac (bc)   
   list<cl_term*>::iterator it = chain.begin();
   cl_term *a = *it;     //a
   cl_term *b = *(++it); //b
   cl_term *c = *(++it); //c
   chain.erase(chain.begin(), ++it); //remove a, b, c from the chain
   
   cl_term* c2 = new cl_term(*c); //copy c
   
   b->chain.push_back(c2); //add second c to b
   
   
   chain.insert(chain.begin(), b); //add (bc2) to the front of the chain
   chain.insert(chain.begin(), c); //add c to the front of the chain
   
   //replace current term with a
   replace_this_with_a(a);
}
//                                                                          
void cl_term::apply_K()
{
   if (term != 'K' && chain.size() < 2)
     {
	cerr<<"Error | cl_term::apply_K | bad term application"<<endl;
     }
   //K a b --> a
   list<cl_term*>::iterator it = chain.begin();
   cl_term *a = *it; //a
   cl_term *b = *(++it); //b
   delete b; //we don't need b
   chain.erase(chain.begin(), ++it); //remove a and b from the chain
   
   //replace current term with a
   replace_this_with_a(a);
}
//                                                                           
void cl_term::apply_I()
{
   if (term != 'I' && chain.size() < 1)
     {
	cerr<<"Error | cl_term::apply_I | bad term application"<<endl;
     }
   //Ia --> a
   list<cl_term*>::iterator it = chain.begin();
   cl_term *a = *it; //a
   chain.erase(it); //remove a from the chain

   //replace current term with a
   replace_this_with_a(a);
}
//                                                                             
void cl_term::apply_B()
{
   if (term != 'B' && chain.size() < 3)
     {
	cerr<<"Error | cl_term::apply_B | bad term application"<<endl;
     }
   //B a b c  a (b c)
   
   list<cl_term*>::iterator it = chain.begin();
   cl_term *a = *it;     //a
   cl_term *b = *(++it); //b
   cl_term *c = *(++it); //c
   chain.erase(chain.begin(), ++it); //remove a, b, c from the chain
      
   b->chain.push_back(c); //add c to b
      
   chain.insert(chain.begin(), b); //add (bc) to the front of the chain
   
   //replace current term with a
   replace_this_with_a(a);
}
//                                                                             
void cl_term::apply_C()
{
   if (term != 'C' && chain.size() < 3)
     {
	cerr<<"Error | cl_term::apply_C | bad term application"<<endl;
     }
   //Cabc acb
   
   list<cl_term*>::iterator it = chain.begin();
   cl_term *a = *it;     //a
   cl_term *b = *(++it); //b
   cl_term *c = *(++it); //c
   chain.erase(chain.begin(), ++it); //remove a, b, c from the chain
      
      
   chain.insert(chain.begin(), b); //add b to the front of the chain
   chain.insert(chain.begin(), c); //add c to the front of the chain
   
   //replace current term with a
   replace_this_with_a(a);
}
//                                                                          
void cl_term::apply_W()
{
   if (term != 'W' && chain.size() < 2)
     {
	cerr<<"Error | cl_term::apply_W | bad term application"<<endl;
     }
   //W a b --> abb
   list<cl_term*>::iterator it = chain.begin();
   cl_term *a = *it;     //a
   cl_term *b = *(++it); //b
   chain.erase(chain.begin(), ++it); //remove a and b from the chain
   
   cl_term* b2 = new cl_term(*b); //copy b
   chain.insert(chain.begin(), b);  //insert b in from of the chain
   chain.insert(chain.begin(), b2); //insert b2 in from of the chain
   
   
   //replace current term with a
   replace_this_with_a(a);
}
//                                                                           
void cl_term::apply_Y()
{
   if (term != 'Y' && chain.size() < 1)
     {
	cerr<<"Error | cl_term::apply_Y | bad term application"<<endl;
     }
   //Ya --> a(Ya)
   list<cl_term*>::iterator it = chain.begin(); //iterator to a
   cl_term *a = *it;     //a
   chain.erase(it);      //remove a from the chain
   cl_term* a2 = new cl_term(*a); //copy a
   
   cl_term* Y  = new cl_term('Y',counter);
   Y->chain.push_back(a2);   //Ya2
   a->chain.push_back(Y);    //we've created a(Ya2)
   
   //replace current term with a
   replace_this_with_a(a);   
}
//                                                                           
void cl_term::replace_this_with_a(cl_term* a)
{
   term = a->term;
   chain.splice(chain.begin(), a->chain);
//   a->chain.clear(); //we copied it, so we should clear it, in order to not remove it
   delete a;
}
//                                                                           
void cl_term::rec_create_term(string s, size_t& pos)
{
   if (s[pos] != '(' || term != 0 || chain.size() != 0)
     {
	cout<<"Error | rec_create_term | internal error 1"<<endl;
	exit(EXIT_FAILURE);
     }
   pos++; //eat leading (
   
   if (s[pos] == '(')
     rec_create_term(s, pos);
   
   //current symbol is not '(' 
   while (s[pos] != ')' && s.size() != pos)
     {
	if (s[pos] == '(') //create new cl_term
	  {
	     cl_term* sub_term = new cl_term(0, counter);
	     sub_term->rec_create_term(s, pos);
	     if (sub_term->term == 0) //this is error
	       {
		  cout<<"Error | rec_create_term | internal error 2"<<endl;
		  exit(EXIT_FAILURE);
	       }
	     chain.push_back(sub_term);
	  }
	else
	  {
	     char c = s[pos++];
	     if (term == 0) //this is head
	       term = c;
	     else
	       {
		  chain.push_back(new cl_term(c, counter));
	       }
	  }
     }   
   if (s[pos] != ')')
     {
	cerr<<"Error |  rec_create_term | missing ')'"<<endl;
	exit(EXIT_FAILURE);
     }
   pos++; //eat tailing ')'
}
//                                                                           
int cl_term::count_nonterm_nodes()
{
  int count = chain.size();
  if (count == 0)
    return 0;
  for (list<cl_term*>::iterator it = chain.begin(); it != chain.end() ; it++)
  {
    if ((*it)->chain.size() != 0)
    {
      count += (*it)->count_nonterm_nodes();
    }
  }
  return count;
}
//                                                                        
int cl_term::get_nonterm_node(cl_term** term, int n)
{
  int count = 0;
  if (n == 0) {
    *term = this;
    return 0;
  }
  *term = NULL;
  if (chain.size() == 0) {
    return 0;
  }
  for (list<cl_term*>::iterator it = chain.begin(); it != chain.end() ; it++) {
    count++;
    if (n <= count) {
      *term = (*it);
      return 0;
    }
    if ((*it)->chain.size() != 0) {
      count += (*it)->get_nonterm_node(term, n-count);
      if (*term != NULL) {
        return 0;
      }
    }
  }
  return count;
}
//                                                                        
void cl_term::crossover_a(cl_term* a)
{
   if (a == this)
     {
	cerr<<"Error | cl_term::crossover | cannot make crossover with selfmem"<<endl;
	exit(EXIT_FAILURE);
     }
   cl_term * term1, * term2;
   int cnt1 = count_nonterm_nodes();
   int cnt2 = a->count_nonterm_nodes();
   //if cnt1 > 1 && cnt2 > 1
   
//   term1->rec_print(cout); cout<<endl;
//   term2->rec_print(cout); cout<<endl;
//   cout<<endl;
   get_nonterm_node(&term1,    random() %(cnt1 + 1));
   a->get_nonterm_node(&term2, random() %(cnt2 + 1) );
   
   term1->chain.swap(term2->chain);
}
//                                                                        
int cl_term::count_subterms()
{
   nsubterms = chain.size();
   for (list<cl_term*>::iterator it = chain.begin(); it != chain.end() ; it++)
     {
	nsubterms += (*it)->count_subterms();
     }
   return nsubterms;
}
//                                                                        
cl_term* cl_term::get_subterm(int num)
{
   if (num < 0)
     {
	cerr<<"Error | cl_term::get_sub_term | num < 0"<<endl;
	exit(EXIT_FAILURE);
     }
   if (num == 0)
     return this;
   num--;
   for (list<cl_term*>::iterator it = chain.begin(); it != chain.end() ; it++)
       {
	  if (num <= (*it)->nsubterms)
	    return (*it)->get_subterm(num);
	  num -= (*it)->nsubterms + 1;
       }
   cerr<<"Error | cl_term::get_sub_term | internal error"<<endl;
   exit(EXIT_FAILURE);
   return NULL;
}
//                                                                        
void cl_term::crossover_s1(cl_term* a)
{
   if (a == this)
     {
	cerr<<"Error | cl_term::crossover_s1 | cannot make crossover with selfmem"<<endl;
	exit(EXIT_FAILURE);
     }
   int cnt1 =    count_subterms();
   int cnt2 = a->count_subterms();

   cl_term *term1 =    get_subterm(random() %(cnt1 + 1));
   cl_term *term2 = a->get_subterm(random() %(cnt2 + 1) );
   
   term1->chain.swap(term2->chain);
   swap(term1->term, term2->term);
}
//                                                                        
void cl_term::echange_mutation(double p, string alphabet)
{
   if ( random()/(double)RAND_MAX < p )
     {
	term = alphabet[random() % alphabet.size()];
     }
   for (list<cl_term*>::iterator it = chain.begin(); it != chain.end() ; it++)
     (*it)->echange_mutation(p, alphabet);
}
//                                                                       
void cl_term::trim_mutation(double p)
{
   for (list<cl_term*>::iterator it = chain.begin(); it != chain.end() ; it++)
     {
	if (random()/(double)RAND_MAX < p)
	  {
	     delete *it;
	     chain.erase(it);
	     return;
	  }
	(*it)->trim_mutation(p);
     }
}
//                                                                       
void cl_term::add_postfix(string postfix)
{
   for(size_t i = 0 ; i < postfix.size() ;i++)
     {
	if (postfix[i] == ')' || postfix[i] == '(')
	  {
	     cerr<<"Error | cl_term::add_postfix | postfix shoudn't contain \'(\' or \')\'"<<endl;
	     exit(EXIT_FAILURE);
	  }
	chain.push_back(new cl_term(postfix[i], counter));
     }
}
//                                                                       
