function scan = make_scandist(Es,Cs,Ep,Cp)
	% scan = make_scandist(Es,Cs,Ep,Cp)
	%
	%	Generates a structure that contains statistical
	%	information about the given scan
	%
	% arguments:
	%
	%	Es,Cs - The mean/cov of the sensor position
	%	Ep,Cp - The mean/cov of the scan point position
	%

	% populate struct with input
	scan.Es = Es;
	scan.Cs = Cs;
	scan.Ep = Ep;
	scan.Cp = Cp;

	% find the unit vector of the scan (from mean to mean)
	scan.u = Ep - Es;
	scan.dist = norm(scan.u);
	scan.u = scan.u / scan.dist;

	% find closest eigenvectors of each point's covariance to
	% the ml ray of the scan
	[Vs,Ds] = eig(Cs);
	[junk,i] = max(abs(Vs'*scan.u));
	scan.eig_s = Vs(:,i);
	if(dot(scan.eig_s, scan.u) < 0)
		scan.eig_s = -1*scan.eig_s;
	end
	scan.eig_s_dot = dot(scan.u, scan.eig_s); 

	[Vp,Dp] = eig(Cp);
	[junk,i] = max(abs(Vp'*scan.u));
	scan.eig_p = Vp(:,i);
	if(dot(scan.eig_p, scan.u) > 0)
		scan.eig_p = -1*scan.eig_p;
	end
	scan.eig_p_dot = dot(scan.u, scan.eig_p);
	
	% get variance of each points' distribution along the
	% ray defined by u (the look vector of the scan)
	scan.varu_s = ( scan.u' * scan.Cs * scan.u );
	scan.varu_p = ( scan.u' * scan.Cp * scan.u );

	% get normalization constant for line
	scan.linenorm = 1 / (1 + (scan.dist ...
		/ sqrt(pi * scan.u' * (scan.Cs + scan.Cp) * scan.u)));

	% compute useful bounds
	scan.xmin = min(scan.Es(1) - 4*scan.Cs(1,1), ...
	                scan.Ep(1) - 4*scan.Cp(1,1));
	scan.xmax = max(scan.Es(1) + 4*scan.Cs(1,1), ...
	                scan.Ep(1) + 4*scan.Cp(1,1));
	scan.ymin = min(scan.Es(2) - 4*scan.Cs(2,2), ...
	                scan.Ep(2) - 4*scan.Cp(2,2));
	scan.ymax = max(scan.Es(2) + 4*scan.Cs(2,2), ...
	                scan.Ep(2) + 4*scan.Cp(2,2));
end
