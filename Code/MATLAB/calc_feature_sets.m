function [normal_features, murmur_features, eval_features] = calc_feature_sets()

% List of files for feature calculations
% fileListNormal = getAllFiles('C:\Users\Michael\Documents\College Stuff\(5) MEng\Design Project\MATLAB\Training B Normal');
fileListNormal = getAllFiles('C:\Users\Michael\Documents\College Stuff\(5) MEng\Design Project\MATLAB\Training_Trim');
fileListMurmur = getAllFiles('C:\Users\Michael\Documents\College Stuff\(5) MEng\Design Project\MATLAB\Btraining_murmur');
% fileListEval = getAllFiles('C:\Users\Michael\Documents\College Stuff\(5)
% MEng\Design Project\MATLAB\Bunlabelledtest\Evaluation');
fileListEval = getAllFiles('C:\Users\Michael\Documents\College Stuff\(5) MEng\Design Project\MATLAB\Bunlabelledtest\Evaluation_Trim');

% Calculate murmur features (training)
for fileIndexMurmur=1:length(fileListMurmur)
    [pathstr, name, ext] = fileparts(fileListMurmur{fileIndexMurmur});
    if strcmp(ext,'.wav')
        filename = fileListMurmur{fileIndexMurmur};
        
        [signal_raw, fs, nbits] = wavread(filename);
        low_rms_percent = calc_low_energy(abs(signal_raw));
        
        signal = miraudio(filename);
        [brightness, centroid, entropy, lowenergy, skewness, kurtosis] = calc_features(signal); 
        
        brightness_all_murmur(fileIndexMurmur) = mirgetdata(brightness);
        centroid_all_murmur(fileIndexMurmur) = mirgetdata(centroid);
        entropy_all_murmur(fileIndexMurmur) = mirgetdata(entropy);
%         lowenergy_all_murmur(fileIndexMurmur) = mirgetdata(lowenergy);
        lowenergy_all_murmur(fileIndexMurmur) = low_rms_percent;
        skewness_all_murmur(fileIndexMurmur) = log10(mirgetdata(skewness));
        kurtosis_all_murmur(fileIndexMurmur) = log10(mirgetdata(kurtosis));
    end
end
murmur_features = struct('brightness',	brightness_all_murmur,  ...
                         'centroid',    centroid_all_murmur,    ...
                         'entropy',     entropy_all_murmur,     ...
                         'lowenergy',   lowenergy_all_murmur,   ...
                         'skewness',    skewness_all_murmur,    ...
                         'kurtosis',    kurtosis_all_murmur);
fid = fopen('mirfeatures_murmur.txt','wt');
write_data = [brightness_all_murmur; centroid_all_murmur; entropy_all_murmur; lowenergy_all_murmur; skewness_all_murmur; kurtosis_all_murmur];
fprintf(fid,'%f\t%f\t%f\t%f\t%f\t%f\n', write_data);
fclose(fid);
                     
% Calculate normal features (training)   
for fileIndexNormal=1:length(fileListNormal)
    [pathstr, name, ext] = fileparts(fileListNormal{fileIndexNormal});
    if strcmp(ext,'.wav')
        filename = fileListNormal{fileIndexNormal};
        
        [signal_raw, fs, nbits] = wavread(filename);
        low_rms_percent = calc_low_energy(abs(signal_raw));
        
        signal = miraudio(filename);
        [brightness, centroid, entropy, lowenergy, skewness, kurtosis] = calc_features(signal); 
        
        brightness_all_normal(fileIndexNormal) = mirgetdata(brightness); 
        centroid_all_normal(fileIndexNormal) = mirgetdata(centroid);
        entropy_all_normal(fileIndexNormal) = mirgetdata(entropy);
%         lowenergy_all_normal(fileIndexNormal) = mirgetdata(lowenergy);
        lowenergy_all_normal(fileIndexNormal) = low_rms_percent;        
        skewness_all_normal(fileIndexNormal) = log10(mirgetdata(skewness));
        kurtosis_all_normal(fileIndexNormal) = log10(mirgetdata(kurtosis));
    end
end
normal_features = struct('brightness',	brightness_all_normal,  ...
                         'centroid',    centroid_all_normal,    ...
                         'entropy',     entropy_all_normal,     ...
                         'lowenergy',   lowenergy_all_normal,   ...
                         'skewness',    skewness_all_normal,    ...
                         'kurtosis',    kurtosis_all_normal);
fid = fopen('mirfeatures_normal.txt','wt');
write_data = [brightness_all_normal; centroid_all_normal; entropy_all_normal; lowenergy_all_normal; skewness_all_normal; kurtosis_all_normal];
fprintf(fid,'%f\t%f\t%f\t%f\t%f\t%f\n', write_data);
fclose(fid);
                     
% Calculate evaluation features
for fileIndexEval=1:length(fileListEval)
    [pathstr, name, ext] = fileparts(fileListEval{fileIndexEval});
    if strcmp(ext,'.wav')
        filename = fileListEval{fileIndexEval};
        
        [signal_raw, fs, nbits] = wavread(filename);
        low_rms_percent = calc_low_energy(abs(signal_raw));
        
        signal = miraudio(filename);
        [brightness, centroid, entropy, lowenergy, skewness, kurtosis] = calc_features(signal);        
        
        brightness_all_eval(fileIndexEval) = mirgetdata(brightness);
        centroid_all_eval(fileIndexEval) = mirgetdata(centroid);
        entropy_all_eval(fileIndexEval) = mirgetdata(entropy);
%         lowenergy_all_eval(fileIndexEval) = mirgetdata(lowenergy);
        lowenergy_all_eval(fileIndexEval) = low_rms_percent;
        skewness_all_eval(fileIndexEval) = log10(mirgetdata(skewness));
        kurtosis_all_eval(fileIndexEval) = log10(mirgetdata(kurtosis));
    end
end
eval_features = struct('brightness',    brightness_all_eval,  ...
                       'centroid',      centroid_all_eval,  ...
                       'entropy',       entropy_all_eval,   ...
                       'lowenergy',     lowenergy_all_eval, ...
                       'skewness',      skewness_all_eval,  ...
                       'kurtosis',      kurtosis_all_eval);
fid = fopen('mirfeatures_eval.txt','wt');
write_data = [brightness_all_eval; centroid_all_eval; entropy_all_eval; lowenergy_all_eval; skewness_all_eval; kurtosis_all_eval];
fprintf(fid,'%f\t%f\t%f\t%f\t%f\t%f\n', write_data);
fclose(fid);
                   
% Calculate features for one signal
function [brightness, centroid, entropy, lowenergy, skewness, kurtosis] = calc_features(signal)
    lowenergy = mirlowenergy(signal);
    brightness = mirbrightness(signal, 'Cutoff', 200);
    entropy = mirentropy(signal);
    centroid = mircentroid(signal);
    skewness = mirskewness(signal);
    kurtosis = mirkurtosis(signal);
    
% Calculate low energy for one signal
function low_energy_percent = calc_low_energy(signal_raw)
    rms = sqrt(sum(signal_raw .^ 2) / length(signal_raw));
    low_rms = ones(1,length(signal_raw));
    for rms_index=1:length(signal_raw)
       if signal_raw(rms_index) < rms
          low_rms(rms_index) = 1;
       else
          low_rms(rms_index) = 0;
       end
    end
    low_energy_percent = sum(low_rms) / length(low_rms);

