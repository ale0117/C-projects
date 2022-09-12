import os

# Please use the two commands below to compile your code.
# Feel free to modify the commands and use any switches you like as long as it compiles on openlab.
# Please do not change the names of the executables.
os.system('gcc -g -Wall "Client Domain/client.c" -o "Client Domain/client" -lcrypto -lssl')
os.system('gcc -g -Wall -pthread "Server Domain/server.c" -o "Server Domain/server" -lcrypto -lssl')

# If you want to compile and test your code in a single python script, uncomment the below two commands.
# Note: please make sure to re-comment or delete the two commands below when you submit your auto_compile.py
# os.system("./Server\ Domain/server 128.195.27.49")
# os.system("./Client\ Domain/client user1_commands.txt 128.195.27.49")
# os.system("./Client\ Domain/client user2_commands.txt 128.195.27.49")



