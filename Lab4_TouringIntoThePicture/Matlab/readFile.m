fileID = fopen('../world.txt','r');
numpollies = fscanf(fileID, 'NUMPOLLIES %d\n', 1)

for i=1:numpollies
	corners = fscanf(fileID, 'SHAPE %d\n', 1);
	if corners == 4
		for j = 1:4
			coord{j} = fscanf(fileID, '%f %f %f %f %f\n', 5);
		end
		name = fscanf(fileID, '%s\n', 1);
	end
end
fclose(fileID);

%     for (int loop = 0; loop < numpollies; loop++) {
%         int type;
%         readstr(filein, oneline);
%         sscanf(oneline, "SHAPE %d\n", &type);
%         if (type == 4) {
%             QUAD qd;
%             for (int vert = 0; vert < type; vert++) {
%                 readstr(filein, oneline);
%                 sscanf(oneline, "%f %f %f %f %f %f %f", &x, &y, &z, &u, &v);
%                 qd.vertex[vert].x = x;
%                 qd.vertex[vert].y = y;
%                 qd.vertex[vert].z = z;
%                 qd.vertex[vert].u = u;
%                 qd.vertex[vert].v = v;
%             }
%             
%             readstr(filein, oneline); 
%             
%             char texfile[80];
%             strcpy(texfile, "data/");
%             strcat(texfile, oneline);
%             trimwhitespace(texfile);
%             
%             LoadGLTexture(texfile, texids[loop]);
%             sector1.quadtexs.push_back(texids[loop]);
%             sector1.quads.push_back(qd);
%         }
%         
%     }
%     fclose(filein);
%    
%     return;
% }