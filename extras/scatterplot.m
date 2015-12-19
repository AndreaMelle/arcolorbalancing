x = load("-ascii", "../bin/points.txt");
figure;
scatter(x(:,1), x(:,2), 0.5 * x(:,3), "green", "filled");
