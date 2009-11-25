//Edwin 11/25/09 carved out of CbcCompareActual
#if defined(_MSC_VER)
// Turn off compiler warning about long names
#  pragma warning(disable:4786)
#endif
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <cfloat>
//#define CBC_DEBUG

#include "CbcMessage.hpp"
#include "CbcModel.hpp"
#include "CbcTree.hpp"
#include "CbcCompareActual.hpp"
#include "CoinError.hpp"
#include "CbcCompareDefault.hpp"
/** Default Constructor

*/
CbcCompareDefault::CbcCompareDefault ()
        : CbcCompareBase(),
        weight_(-1.0),
        saveWeight_(0.0),
        cutoff_(COIN_DBL_MAX),
        bestPossible_(-COIN_DBL_MAX),
        numberSolutions_(0),
        treeSize_(0),
        breadthDepth_(5)
{
    test_ = this;
}

// Constructor with weight
CbcCompareDefault::CbcCompareDefault (double weight)
        : CbcCompareBase(),
        weight_(weight) ,
        saveWeight_(0.0),
        cutoff_(COIN_DBL_MAX),
        bestPossible_(-COIN_DBL_MAX),
        numberSolutions_(0),
        treeSize_(0),
        breadthDepth_(5)
{
    test_ = this;
}


// Copy constructor
CbcCompareDefault::CbcCompareDefault ( const CbcCompareDefault & rhs)
        : CbcCompareBase(rhs)

{
    weight_ = rhs.weight_;
    saveWeight_ = rhs.saveWeight_;
    cutoff_ = rhs.cutoff_;
    bestPossible_ = rhs.bestPossible_;
    numberSolutions_ = rhs.numberSolutions_;
    treeSize_ = rhs.treeSize_;
    breadthDepth_ = rhs.breadthDepth_;
}

// Clone
CbcCompareBase *
CbcCompareDefault::clone() const
{
    return new CbcCompareDefault(*this);
}

// Assignment operator
CbcCompareDefault &
CbcCompareDefault::operator=( const CbcCompareDefault & rhs)
{
    if (this != &rhs) {
        CbcCompareBase::operator=(rhs);
        weight_ = rhs.weight_;
        saveWeight_ = rhs.saveWeight_;
        cutoff_ = rhs.cutoff_;
        bestPossible_ = rhs.bestPossible_;
        numberSolutions_ = rhs.numberSolutions_;
        treeSize_ = rhs.treeSize_;
        breadthDepth_ = rhs.breadthDepth_;
    }
    return *this;
}

// Destructor
CbcCompareDefault::~CbcCompareDefault ()
{
}

// Returns true if y better than x
bool
CbcCompareDefault::test (CbcNode * x, CbcNode * y)
{
#if 0
    // always choose *smallest* depth if one or both <= breadthDepth_
    int depthX = x->depth();
    int depthY = y->depth();
    if (depthX <= breadthDepth_ || depthY <= breadthDepth_) {
        if (depthX != depthY)
            return depthX > depthY;
        else
            return equalityTest(x, y); // so ties will be broken in consistent manner
    }
    if (weight_ == -1.0 || weight_ == -3.0) {
        int adjust =  (weight_ == -3.0) ? 10000 : 0;
        // before solution
        /*printf("x %d %d %g, y %d %d %g\n",
           x->numberUnsatisfied(),x->depth(),x->objectiveValue(),
           y->numberUnsatisfied(),y->depth(),y->objectiveValue()); */
        if (x->numberUnsatisfied() > y->numberUnsatisfied() + adjust) {
            return true;
        } else if (x->numberUnsatisfied() < y->numberUnsatisfied() - adjust) {
            return false;
        } else {
            int depthX = x->depth();
            int depthY = y->depth();
            if (depthX != depthY)
                return depthX < depthY;
            else
                return equalityTest(x, y); // so ties will be broken in consistent manner
        }
    } else {
        // after solution
        double weight = CoinMax(weight_, 0.0);
        double testX =  x->objectiveValue() + weight * x->numberUnsatisfied();
        double testY = y->objectiveValue() + weight * y->numberUnsatisfied();
        if (testX != testY)
            return testX > testY;
        else
            return equalityTest(x, y); // so ties will be broken in consistent manner
    }
#else
    //weight_=0.0;
    if ((weight_ == -1.0 && (y->depth() > breadthDepth_ && x->depth() > breadthDepth_)) || weight_ == -3.0 || weight_ == -2.0) {
        int adjust =  (weight_ == -3.0) ? 10000 : 0;
        // before solution
        /*printf("x %d %d %g, y %d %d %g\n",
           x->numberUnsatisfied(),x->depth(),x->objectiveValue(),
           y->numberUnsatisfied(),y->depth(),y->objectiveValue()); */
        if (x->numberUnsatisfied() > y->numberUnsatisfied() + adjust) {
            return true;
        } else if (x->numberUnsatisfied() < y->numberUnsatisfied() - adjust) {
            return false;
        } else {
            int depthX = x->depth();
            int depthY = y->depth();
            if (depthX != depthY)
                return depthX < depthY;
            else
                return equalityTest(x, y); // so ties will be broken in consistent manner
        }
    } else {
        // always choose *greatest* depth if both <= breadthDepth_ otherwise <= breadthDepth_ if just one
        int depthX = x->depth();
        int depthY = y->depth();
        /*if ((depthX==4&&depthY==5)||(depthX==5&&depthY==4))
          printf("X %x depth %d, Y %x depth %d, breadth %d\n",
          x,depthX,y,depthY,breadthDepth_);*/
        if (depthX <= breadthDepth_ || depthY <= breadthDepth_) {
            if (depthX <= breadthDepth_ && depthY <= breadthDepth_) {
                if (depthX != depthY) {
                    return depthX < depthY;
                }
            } else {
                assert (depthX != depthY) ;
                return depthX > depthY;
            }
        }
        // after solution ?
#define THRESH2 0.999
#define TRY_THIS 0
#if TRY_THIS==0
        double weight = CoinMax(weight_, 1.0e-9);
        double testX =  x->objectiveValue() + weight * x->numberUnsatisfied();
        double testY = y->objectiveValue() + weight * y->numberUnsatisfied();
#elif TRY_THIS==1
    /* compute what weight would have to be to hit target
       then reverse sign as large weight good */
    double target = (1.0 - THRESH2) * bestPossible_ + THRESH2 * cutoff_;
    double weight;
    weight = (target - x->objectiveValue()) /
             static_cast<double>(x->numberUnsatisfied());
    double testX = - weight;
    weight = (target - y->objectiveValue()) /
             static_cast<double>(y->numberUnsatisfied());
    double testY = - weight;
#elif TRY_THIS==2
    // Use estimates
    double testX = x->guessedObjectiveValue();
    double testY = y->guessedObjectiveValue();
#elif TRY_THIS==3
#define THRESH 0.95
    // Use estimates
    double testX = x->guessedObjectiveValue();
    double testY = y->guessedObjectiveValue();
    if (x->objectiveValue() - bestPossible_ > THRESH*(cutoff_ - bestPossible_))
        testX *= 2.0; // make worse
    if (y->objectiveValue() - bestPossible_ > THRESH*(cutoff_ - bestPossible_))
        testY *= 2.0; // make worse
#endif
        if (testX != testY)
            return testX > testY;
        else
            return equalityTest(x, y); // so ties will be broken in consistent manner
    }
#endif
}
// This allows method to change behavior as it is called
// after each solution
void
CbcCompareDefault::newSolution(CbcModel * model,
                               double objectiveAtContinuous,
                               int numberInfeasibilitiesAtContinuous)
{
    cutoff_ = model->getCutoff();
    if (model->getSolutionCount() == model->getNumberHeuristicSolutions() &&
            model->getSolutionCount() < 5 && model->getNodeCount() < 500)
        return; // solution was got by rounding
    // set to get close to this solution
    double costPerInteger =
        (model->getObjValue() - objectiveAtContinuous) /
        static_cast<double> (numberInfeasibilitiesAtContinuous);
    weight_ = 0.95 * costPerInteger;
    saveWeight_ = 0.95 * weight_;
    numberSolutions_++;
    //if (numberSolutions_>5)
    //weight_ =0.0; // this searches on objective
}
// This allows method to change behavior
bool
CbcCompareDefault::every1000Nodes(CbcModel * model, int numberNodes)
{
#if 0
    // was
    if (numberNodes > 10000)
        weight_ = 0.0; // this searches on objective
    // get size of tree
    treeSize_ = model->tree()->size();
#else
    double saveWeight = weight_;
    int numberNodes1000 = numberNodes / 1000;
    if (numberNodes > 10000) {
        weight_ = 0.0; // this searches on objective
        // but try a bit of other stuff
        if ((numberNodes1000 % 4) == 1)
            weight_ = saveWeight_;
    } else if (numberNodes == 1000 && weight_ == -2.0) {
        weight_ = -1.0; // Go to depth first
    }
    // get size of tree
    treeSize_ = model->tree()->size();
    if (treeSize_ > 10000) {
        int n1 = model->solver()->getNumRows() + model->solver()->getNumCols();
        int n2 = model->numberObjects();
        double size = n1 * 0.1 + n2 * 2.0;
        // set weight to reduce size most of time
        if (treeSize_*(size + 100.0) > 5.0e7)
            weight_ = -3.0;
        else if ((numberNodes1000 % 4) == 0 && treeSize_*size > 1.0e6)
            weight_ = -1.0;
        else if ((numberNodes1000 % 4) == 1)
            weight_ = 0.0;
        else
            weight_ = saveWeight_;
    }
#endif
    //return numberNodes==11000; // resort if first time
    return (weight_ != saveWeight);
}

// Create C++ lines to get to current state
void
CbcCompareDefault::generateCpp( FILE * fp)
{
    CbcCompareDefault other;
    fprintf(fp, "0#include \"CbcCompareActual.hpp\"\n");
    fprintf(fp, "3  CbcCompareDefault compare;\n");
    if (weight_ != other.weight_)
        fprintf(fp, "3  compare.setWeight(%g);\n", weight_);
    fprintf(fp, "3  cbcModel->setNodeComparison(compare);\n");
}
