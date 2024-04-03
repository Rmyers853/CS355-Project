# CS355-Project
Setup: Two users agree on who will use the client program and who will use the server program. Both each have 5 files that they would like to compare with the other user to make sure they don't share any same file, without the other user learning the contents of any file. There is also an adversary listening to communication between the two users, so we need to make sure that they also can't learn any contents of any file sent.  
Implementation: Both users generate a secret key so that neither can decrypt a file with just their key. Next, they send the encrypted files using El Gamal to prevent the adversary from learning any of the files contents. Once each user recieves the other users' 5 files, they encrypt the files with their secret key and send the files back to the original user to be used for comparision. If ((m1)^sk1)^sk2 = ((m2)^sk2)^sk1, then m1 = m2. Here is a link to see a visual of this setup: https://docs.google.com/presentation/d/1moNDE1jV59UsSLcylJKZulBxRSpu1AgoScWHvCRO3-4/edit?usp=sharing  
Also, here is a good resource for how El Gamal works and how I performed the encryption necessary in this program: https://www.geeksforgeeks.org/elgamal-encryption-algorithm/  
To run in terminal, type the commands:  
g++ -o gamal gamal.cpp -o client client.cpp  
g++ -o gamal gamal.cpp -o server server.cpp  
./server  
./client  
