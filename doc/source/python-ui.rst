.. _python-ui:

Python UI
=========

Explain a little how to use the python interface. mention pybind11. 
mention its the preferred way to use compwa!
So to construct a model, we do not expect to use or need a dataset, and vice versa. 
What is the way to use ComPWA:
-First you readin your datasets. Alternatively you can generate new ones.
-However for that you need a intensity or model first.
The kinematics and the intensity are created hand in hand using the builder.
-If you have not read in data yet you can do that now, 
or you can generate a dataset with Hit&Miss.
-afterwards its rather straight forward. create an estimator
 and give it the intensity and the data.
-the estimator is given to the optimizer.

That's it. It would be straight forward to create the kinematics and the model first.
The only problem is the integration decorators that need a dataset. 
 So we need to give the decorators some integration strategy interface, 
 so that the model and kinematics can be created first. Then the integration strategy
  can be actually initialized with a dataset. alternatively we can define a dataset
   interface, which is empty in the beginning. that is passed to the builder. 
   then later on we can add actual data to the 
