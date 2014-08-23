murmur_data = {murmur_features.brightness, murmur_features.centroid, murmur_features.entropy, murmur_features.lowenergy, log10(murmur_features.skewness), log10(murmur_features.kurtosis)};
normal_data = {normal_features.brightness, normal_features.centroid, normal_features.entropy, normal_features.lowenergy, log10(normal_features.skewness), log10(normal_features.kurtosis)};
feature_names = {'brightness', 'centroid', 'entropy', 'low energy', 'skewness', 'kurtosis'};
num_features = length(murmur_data);

% for index=1:num_features
%     figure;
%     hist(murmur_data{index},20)
%     h = findobj(gca,'Type','patch');
%     set(h,'FaceColor','r','EdgeColor','w','facealpha',0.75)
%     hold on;
%     hist(normal_data{index},20)
%     h1 = findobj(gca,'Type','patch');
%     set(h1,'facealpha',0.75);
%     hold off;
%     title(feature_names{index});
% end    

figure(1);
hist(murmur_data{4},20)
h = findobj(gca,'Type','patch');
set(h,'FaceColor','r','EdgeColor','w','facealpha',0.75)
hold on;
hist(normal_data{4},20)
h1 = findobj(gca,'Type','patch');
set(h1,'facealpha',0.75);
hold off;
title('Low Energy Decision Plane');
legend('Murmur', 'Normal');
ylabel('Frequency');
xlabel('Low Energy Rate');
line([0.8633 0.8633], [0 12], 'Color', 'g');

% fid = fopen('features_mir_murmur.txt','wt');
% write_data = [brightness_all_murmur; centroid_all_murmur; entropy_all_murmur; lowenergy_all_murmur; log10(skewness_all_murmur); log10(kurtosis_all_murmur)];
% fprintf(fid,'%f\t%f\t%f\t%f\t%f\t%f\n', write_data);
% fclose(fid);
% 
% fid = fopen('features_mir_normal.txt','wt');
% write_data = [brightness_all_normal; centroid_all_normal; entropy_all_normal; lowenergy_all_normal; log10(skewness_all_normal); log10(kurtosis_all_normal)];
% fprintf(fid,'%f\t%f\t%f\t%f\t%f\t%f\n', write_data);
% fclose(fid);