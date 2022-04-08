NUMBER=10
for i in {0..10}
  do 
     ../build/odbc/bin/ignite-odbc-tests
     echo $?
 done
