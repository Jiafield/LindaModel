Nuts
====
Linda is a parallel processing model with a global tuple space. A tuple is a list of items of any data types, to simplify the implementation, here I only support integer, double, and string. The syntax of the commands are: 

 out(<tuple>); # put passive <tuple> to the tuple space 
 eval(<expressions>); # put live <tuple> by evaluate expressions 
 in(<pattern>); # blocking pattern match tuples from tuple space 
 rd(<pattern>); # same as in, but keep tuples in tuple space 
 inp(<pattern>); # return true/false, non-blocking version of in 
 rdp(<pattern>); # return true/false, non-blocking version of rd 
 for () { } # for loop 
 if (<expression>) { } [else if () { } [else { }]] # for loop 
 dump(); # dump all tuples in the global tuple space 
 
The <pattern> is a list of items, and each item can be of any data type or a variable (preceded by question marks, e.g., ?i, ?f, ?s) of certain type.
