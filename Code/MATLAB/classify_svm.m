function [SVMstruct, eval_groups] = classify_svm(normal_features, murmur_features, eval_features)

% Create SVM data for training and evaluation
normal_data = [                             ...
%                normal_features.centroid'    ...
%                normal_features.brightness'  ...
%                normal_features.entropy'     ...
               normal_features.lowenergy'   ...
%                normal_features.skewness'    ...
%                normal_features.kurtosis'    ...
              ];
murmur_data = [                             ...
%                murmur_features.centroid'    ...
%                murmur_features.brightness'  ...
%                murmur_features.entropy'     ...
               murmur_features.lowenergy'   ...
%                murmur_features.skewness'    ...
%                murmur_features.kurtosis'    ...
              ];  
eval_data = [                           ...
%              eval_features.centroid'    ...
%              eval_features.brightness'  ...
%              eval_features.entropy'     ...
             eval_features.lowenergy'   ...
%              eval_features.skewness'    ...
%              eval_features.kurtosis'    ...
            ];          
%training_data = [normal_data; murmur_data];
% training_data = [normal_data; murmur_data];
% training_groups(1:151)=0; training_groups(152:217)=1;
% training_data = [normal_data(1:66); murmur_data];
% training_groups(1:66)=0; training_groups(67:132)=1;
training_data = [normal_data; murmur_data];
training_groups(1:83)=0; training_groups(84:149)=1;

% Perform SVM training
SVMstruct = svmtrain(training_data, training_groups);

% Perform SVM classification
eval_groups = svmclassify(SVMstruct, eval_data);