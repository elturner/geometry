function animate_poses(poses, figureNum)

if(~exist('figureNum','var') || isempty(figureNum))
   figureNum = figure;
else
   figure(figureNum) 
end

plot3(poses(1,:),poses(2,:),poses(3,:),'k')
hold on
axis equal

Rned2enu = rpy2rot(180,0,90,'d');
for i = 1:1:size(poses,2)
    R = rpy2rot(poses(4:6,i));
    R = Rned2enu*R;
    
    xaxis = R*[1;0;0];
    yaxis = R*[0;1;0];
    zaxis = R*[0;0;1];
    h1 = plot3([0 xaxis(1)]+poses(1,i),[0 xaxis(2)]+poses(2,i),[0 xaxis(3)]+poses(3,i),'r');
    h2 = plot3([0 yaxis(1)]+poses(1,i),[0 yaxis(2)]+poses(2,i),[0 yaxis(3)]+poses(3,i),'g');
    h3 = plot3([0 zaxis(1)]+poses(1,i),[0 zaxis(2)]+poses(2,i),[0 zaxis(3)]+poses(3,i),'b');
    
    pause(0.1);
    delete(h1);
    delete(h2);
    delete(h3);
    
end

end