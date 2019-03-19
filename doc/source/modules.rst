C++ modules
===========


Ok here we can ask ourselves, what the "Core" library is really needed for.
I think all of the classes or files belong to one of the 4 groups.


The 4 main component groups in ComPWA are:

-Data
-Physics/Models
-Esimators
-Optimaizers

(There is one more group, Tools. It contains tools that do not belong specifically to one of these groups)


The data component group is responsible for:

- transformation of data
- data IO
- data generation (generators should be moved from tools to here)


The Physics component group consists of several subcomponents:

- definitions of all physics models (helicty formalism, ...)
  -> they use the intensity and amplitude interface
  -> they need the data to be evaluated, so as an input
- definition of kinematics class and its interface
  -> they are responsible for transforming Event ../Tools/Plotting/ROOT/DalitzPlot.cppbased data to the kinematic variables
     that are needed by the underlying theory of a model.
  -> they take data as an input and output


The estimator is responsible for determining the "closeness" of the model to the data.
It needs both the data and the data, obviously.


The optimizer needs the estimator, and a set of fit parameters that are obtained from the model.