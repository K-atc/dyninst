/*
 * Copyright (c) 2007-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if !defined(AST_H)
#define AST_H

#include <vector>
#include <string>
#include <sstream>
#include <dyn_detail/boost/shared_ptr.hpp>
#include <dyn_detail/boost/enable_shared_from_this.hpp>
#include "util.h"

namespace Dyninst {

// We fully template the three types of nodes we have so that
// users can specify their own. This basically makes the AST
// a fully generic class. 
//
// TODO: do we want Variable and Constant to be different classes?
// I'm using the Absloc case as the basis here; EAX and '5' are
// very different things...
//
// Possible fourth template type: Type
// though I'm currently arguing that Type is an artifact of the
// Eval method you apply here. 
// ... and are Eval methods independent of Operation/Variable/Constant?
// I think they are...x

class ASTVisitor;  

 // For this to work, the ASTVisitor has to have a virtual
 // visit() method for every instantiation of an AST-typed
 // class. Yes, this means that if you add an AST class
 // somewhere else you have to come back and put it in here. 
 // Well, if you want to run a visitor over it, that is.
 class AST;

 // SymEval...
 namespace SymbolicEvaluation {
 class BottomAST;
 class ConstantAST;
 class VariableAST;
 class RoseAST;
 };
 // Stack analysis...
 class StackAST;

 // InsnAPI...

 // Codegen...

 // Other...
 class InputVariableAST;

 class ASTVisitor {
 public:
   typedef dyn_detail::boost::shared_ptr<AST> ASTPtr;
   virtual ASTPtr visit(AST *) = 0;
   virtual ASTPtr visit(SymbolicEvaluation::BottomAST *) = 0;
   virtual ASTPtr visit(SymbolicEvaluation::ConstantAST *) = 0;
   virtual ASTPtr visit(SymbolicEvaluation::VariableAST *) = 0;
   virtual ASTPtr visit(SymbolicEvaluation::RoseAST *) = 0;
   virtual ASTPtr visit(StackAST *) = 0;
   virtual ASTPtr visit(InputVariableAST *) = 0;

   virtual ~ASTVisitor() {};
 };


#define DEF_AST_LEAF_TYPE(name, type)					\
class name : public AST {						\
 public:								\
 typedef dyn_detail::boost::shared_ptr<name> Ptr;			\
 static Ptr create(type t) { return Ptr(new name(t)); }			\
 virtual ~name() {};							\
 virtual const std::string format() const {				\
   std::stringstream ret;						\
   ret << "<" << t_ << ">";						\
   return ret.str();							\
 }									\
 virtual AST::Ptr accept(ASTVisitor *v) { return v->visit(this); }	\
 virtual ID getID() const { return V_##name; }				\
  static Ptr convert(AST::Ptr a) {					\
    return ((a->getID() == V_##name) ? dyn_detail::boost::static_pointer_cast<name>(a) : Ptr()); \
  }									\
  const type &val() const { return t_; }				\
 private:								\
 name(type t) : t_(t) {};						\
 virtual bool isStrictEqual(const AST &rhs) const {			\
   const name &other(dynamic_cast<const name&>(rhs));			\
   return t_ == other.t_;						\
 }									\
 const type t_;								\
 };									\

#define DEF_AST_INTERNAL_TYPE(name, type)				\
class name : public AST {						\
 public:								\
  typedef dyn_detail::boost::shared_ptr<name> Ptr;			\
  virtual ~name() {};							\
  static Ptr create(type t, AST::Ptr a) { return Ptr(new name(t, a)); }	\
  static Ptr create(type t, AST::Ptr a, AST::Ptr b) { return Ptr(new name(t, a, b)); } \
  static Ptr create(type t, AST::Ptr a, AST::Ptr b, AST::Ptr c) { return Ptr(new name(t, a, b, c)); } \
  static Ptr create(type t, Children c) { return Ptr(new name(t, c)); }	\
  virtual const std::string format() const {				\
    std::stringstream ret;						\
    ret << t_ << "(";                                                   \
    for (Children::const_iterator i = kids_.begin(); i != kids_.end(); ++i) {	\
      ret << (*i)->format() << ",";					\
    }									\
    ret << ")";								\
    return ret.str();							\
  }									\
  virtual AST::Ptr child(unsigned i) const { return kids_[i];}		\
  virtual unsigned numChildren() const { return kids_.size();}		\
  virtual AST::Ptr accept(ASTVisitor *v) { return v->visit(this); }	\
  virtual ID getID() const { return V_##name; }				\
  static Ptr convert(AST::Ptr a) {					\
    return ((a->getID() == V_##name) ? dyn_detail::boost::static_pointer_cast<name>(a) : Ptr()); \
  }									\
  const type &val() const { return t_; }				\
  void setChild(int i, AST::Ptr a) { kids_[i] = a; };			\
 private:								\
 name(type t, AST::Ptr a) : t_(t) { kids_.push_back(a); };		\
 name(type t, AST::Ptr a, AST::Ptr b) : t_(t) {				\
    kids_.push_back(a);							\
    kids_.push_back(b);							\
  };									\
 name(type t, AST::Ptr a, AST::Ptr b, AST::Ptr c) : t_(t) {		\
    kids_.push_back(a);							\
    kids_.push_back(b);							\
    kids_.push_back(c);							\
  };									\
 name(type t, Children kids) : t_(t), kids_(kids) {};			\
  virtual bool isStrictEqual(const AST &rhs) const {			\
    const name &other(dynamic_cast<const name&>(rhs));			\
    return ((t_ == other.t_) && (kids_ == other.kids_));		\
  }									\
  const type t_;							\
  Children kids_;							\
 };									\

class AST : public dyn_detail::boost::enable_shared_from_this<AST> {
 public:

  // This is a global list of all AST types, including those that are not
  // yet implemented. The format is a "V_" string prepending the class name.
  // If you add an AST type you should update this list.

  typedef enum {
    V_AST,
    // SymEval
    V_BottomAST,
    V_ConstantAST,
    V_VariableAST,
    V_RoseAST,
    // Stack analysis
    V_StackAST,
    V_InputVariableAST } ID;

  typedef dyn_detail::boost::shared_ptr<AST> Ptr;
  typedef std::vector<AST::Ptr> Children;      

  AST() {};
  virtual ~AST() {};
  
  bool operator==(const AST &rhs) const {
    // make sure rhs and this have the same type
    return((typeid(*this) == typeid(rhs)) && isStrictEqual(rhs));
  }

  virtual unsigned numChildren() const { return 0; }		       

  virtual AST::Ptr child(unsigned) const {				
    assert(0);								
    return AST::Ptr();							
  }								       

  virtual const std::string format() const = 0;

  // Substitutes every occurrence of a with b in
  // AST in. Returns a new AST. 

  static AST::Ptr substitute(AST::Ptr in, AST::Ptr a, AST::Ptr b); 

  virtual ID getID() const { return V_AST; };

  // VISITOR wooo....
  virtual Ptr accept(ASTVisitor *v) { return v->visit(this); }

  Ptr ptr() { return shared_from_this(); }

  virtual void setChild(int, AST::Ptr) {
    assert(0);
  };

 protected:
  virtual bool isStrictEqual(const AST &rhs) const = 0;
};

}
#endif // AST_H

