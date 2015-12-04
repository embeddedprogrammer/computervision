function addSurface(bim, bim_alpha, pg1, pg2, pg3, pg4)
	%Change of variables to convert C++ file ordering to matlab ordering.
	p1 = pg1;
	p2 = pg4;
	p3 = pg2;
	p4 = pg3;

	% define a surface in 3D (need at least 6 points, for some reason)
	planex = [p1.x  (p1.x + p2.x) / 2  p2.x; p3.x  (p3.x + p4.x) / 2  p4.x];
	planey = [p1.y  (p1.y + p2.y) / 2  p2.y; p3.y  (p3.y + p4.y) / 2  p4.y];
	planez = [p1.z  (p1.z + p2.z) / 2  p2.z; p3.z  (p3.z + p4.z) / 2  p4.z];

	% create the surface and texturemap it with a given image
	warp(planex,planey,planez,bim);
	% some alpha-channel magic to make things transparent
	alpha(bim_alpha);
	alpha('texture');
end