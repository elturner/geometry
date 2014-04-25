function [] = render_stats(M)
	% render_stats(M)
	%
	%	Will render the statistics information about
	%	the leafs in an octree
	%
	% arguments:
	%
	%	M -	A matrix representation of the probability value
	%		and uncertainty estimate for each leaf in the
	%		octree.  M should be of size Nx2, where there are
	%		N leafs, the first column represents probability
	%		values, and the second column represents std. dev.
	%		estimates for those probability values.
	%

	% prepare figure
	figure;
	hold all;

	% determine which values are uncertain
	sig_coef = 5; % number of sigmas that represent uncertainty
	uc_flag = abs(M(:,1) - 0.5) < sig_coef*M(:,2); % in uncertain zone?

	% render values
	plot(M(~uc_flag,1), M(~uc_flag,2), 'b.');
	plot(M(uc_flag,1), M(uc_flag,2), 'r.');
	
	% render bounds
	plot(0.5*(1 + [0 sig_coef]), [0 0.5], 'k-');
	plot(0.5*(1 - [0 sig_coef]), [0 0.5], 'k-');

	% render legend
	legend('Points with high conf', 'Points with low conf', ...
		[num2str(sig_coef),' \sigma bounds']);
	xlabel('Probability a voxel is interior', 'FontSize', 14);
	ylabel('Std. Dev. of Probability Estimate', 'FontSize', 14);
	title([num2str(round(100*sum(uc_flag)/length(uc_flag))), ...
		'% of voxels have uncertain labels'],'Fontsize',18);
end
