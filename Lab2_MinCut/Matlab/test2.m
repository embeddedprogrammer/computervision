tic
A = rand(256, 256, 256);
B = rand(3, 3, 3);
A = convn(A, B, 'same');
A = convn(A, B, 'same');
A = convn(A, B, 'same');
toc