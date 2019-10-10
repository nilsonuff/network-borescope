using System;
using System.Collections.Generic;
using System.IO;

using Microsoft.ML;
using Microsoft.ML.Data;
using Microsoft.ML.Trainers;

using DataStructures;

//using Microsoft.ML.Trainers.FastTree;


namespace tc
{
    internal static class Program
   {
        //private static string BaseDatasetsRelativePath = @"../../../Data";
        enum PredictionAlg {
            Ols, FastTree, LbfgsPoisson, Gam, Sdca, OnlineGradientDescent
        };

        private static string InFileRelativePath = "ts-in.txt";
        private static string InFilePath = GetAbsolutePath(InFileRelativePath);

        private static string OutFileRelativePath = "ts-out.txt";
        private static string OutFilePath = GetAbsolutePath(OutFileRelativePath);

        private static MLContext mlContext;

        static void Main(string[] args) {

            // deleta o arquivo de saida
            File.Create(OutFilePath).Close();

            if (args.Length <= 0) return;

            // Create MLContext to be shared across the model creation workflow objects 
            mlContext = new MLContext();

            //Load the data into IDataView.
            IDataView dataView;

            var algname = args[0];

            if (algname.StartsWith("Detect")) {
                dataView = mlContext.Data.LoadFromTextFile<TSData>(path: InFilePath, hasHeader: false, separatorChar: ',');
                if (algname == "Detect.IidSpike") {
                    int confidence = 95;
                    int historyLength = 9;
                    if (args.Length > 1) {
                        confidence = Int32.Parse(args[1]);
                    }
                    if (args.Length > 2) {
                        historyLength = Int32.Parse(args[2]);
                    }
                    DetectSpike(dataView, 
                        confidence, historyLength);
               

                } else if (algname == "Detect.SpikeBySsa") {
                    int confidence = 95;
                    int historyLength = 9;
                    int trainingWindowSize = 15;
                    int seasonalityWindowSize = 3;

                    if (args.Length > 1) { confidence = Int32.Parse(args[1]); }
                    if (args.Length > 2) { historyLength = Int32.Parse(args[2]); }
                    if (args.Length > 3) { trainingWindowSize = Int32.Parse(args[3]); }
                    if (args.Length > 4) { seasonalityWindowSize = Int32.Parse(args[4]); }

                    if (trainingWindowSize < 3 * seasonalityWindowSize) {
                        trainingWindowSize = 3 * seasonalityWindowSize; 
                    }

                    DetectSpikeBySsa(dataView, 
                        confidence, historyLength,
                        trainingWindowSize,seasonalityWindowSize);

                } else if (algname == "Detect.AnomalyBySrCnn") {
                    int windowSize = 16;
                    int backAddWindowSize = 5;
                    int lookaheadWindowSize = 5;
                    int averageingWindowSize = 3;
                    int judgementWindowSize = 8;
                    double threshold = 0.35;

                    if (args.Length > 1) { windowSize = Int32.Parse(args[1]); }
                    if (args.Length > 2) { backAddWindowSize = Int32.Parse(args[2]); }
                    if (args.Length > 3) { lookaheadWindowSize = Int32.Parse(args[3]); }
                    if (args.Length > 4) { averageingWindowSize = Int32.Parse(args[4]); }
                    if (args.Length > 5) { judgementWindowSize = Int32.Parse(args[5]); }
                    if (args.Length > 6) { threshold = Double.Parse(args[5]); }

                    DetectAnomalyBySrCnn(dataView, 
                        windowSize, backAddWindowSize,
                        lookaheadWindowSize, averageingWindowSize,
                        judgementWindowSize, threshold);
                }


            } else if (args[0].StartsWith("Prediction")) {
                int second = 1;
                int len = 1;
                if (args.Length > 1) { second = Int32.Parse(args[1]); }
                if (args.Length > 2) { len = Int32.Parse(args[2]); }

                dataView = mlContext.Data.LoadFromTextFile<SecValData>(path: InFilePath, hasHeader: false, separatorChar: ',');
                var alg = PredictionAlg.Ols;

                if (algname == "Prediction" || algname == "Prediction.Ols") {
                    alg = PredictionAlg.Ols;

                } else if (algname == "Prediction.FastTree" || algname == "Prediction.FT") {
                    alg = PredictionAlg.FastTree;

                } else if (algname == "Prediction.Poisson" || algname == "Prediction.LbfgsPoisson" ) {
                    alg = PredictionAlg.LbfgsPoisson;
 
                } else if (algname == "Prediction.Gam" ) {
                    alg = PredictionAlg.Gam;
/* Not used yet
                } else if (algname == "Prediction.OGD" || algname == "Prediction.OnlineGradientDescent" ) {
                    alg = PredictionAlg.OnlineGradientDescent;
*/
                } else if (algname == "Prediction.Sdca" ) {
                    alg = PredictionAlg.Sdca;
                }

                doPrediction(alg, dataView, second, len);

            } else {
                return;
            }
        }

        //====================================================
        //
        //  Regressions
        //
        //====================================================

        /************************
         *
         *
         */
        static PredictionEngine<SecValData, SecValDataPrediction> Prediction_Ols(IDataView dataView) {

            var pipeline = mlContext
                    .Transforms
                    .CopyColumns(outputColumnName: "Label", inputColumnName:"Value")
                    .Append(mlContext.Transforms.Concatenate("Features", "Seconds"))
                    .Append(mlContext.Regression.Trainers.Ols());
            
            var model = pipeline.Fit(dataView);

            return mlContext.Model.CreatePredictionEngine<SecValData, SecValDataPrediction>(model);
        }

        /************************
         *
         *
         */
        static PredictionEngine<SecValData, SecValDataPrediction> Prediction_FastTree(IDataView dataView) {
            var pipeline = mlContext
                    .Transforms
                    .CopyColumns(outputColumnName: "Label", inputColumnName:"Value")
                    .Append(mlContext.Transforms.Concatenate("Features", "Seconds"))
                    .Append(mlContext.Regression.Trainers.FastTree());
            
            var model = pipeline.Fit(dataView);

            return mlContext.Model.CreatePredictionEngine<SecValData, SecValDataPrediction>(model);
        }

        /************************
         *
         *
         */
        static PredictionEngine<SecValData, SecValDataPrediction> Prediction_LbfgsPoisson(IDataView dataView) {

            var pipeline = mlContext
                    .Transforms
                    .CopyColumns(outputColumnName: "Label", inputColumnName:"Value")
                    .Append(mlContext.Transforms.Concatenate("Features", "Seconds"))
                    .Append(mlContext.Regression.Trainers.LbfgsPoissonRegression());
            
            var model = pipeline.Fit(dataView);

            return mlContext.Model.CreatePredictionEngine<SecValData, SecValDataPrediction>(model);
        }

        /************************
         *
         *
         */
        static PredictionEngine<SecValData, SecValDataPrediction> Prediction_Gam(IDataView dataView) {

            var pipeline = mlContext
                    .Transforms
                    .CopyColumns(outputColumnName: "Label", inputColumnName:"Value")
                    .Append(mlContext.Transforms.Concatenate("Features", "Seconds"))
                    .Append(mlContext.Regression.Trainers.Gam());
            
            var model = pipeline.Fit(dataView);

            return mlContext.Model.CreatePredictionEngine<SecValData, SecValDataPrediction>(model);
        }


        /************************
         *
         *
         */
        static PredictionEngine<SecValData, SecValDataPrediction> Prediction_Sdca(IDataView dataView) {

            var pipeline = mlContext
                    .Transforms
                    .CopyColumns(outputColumnName: "Label", inputColumnName:"Value")
                    .Append(mlContext.Transforms.Concatenate("Features", "Seconds"))
                    .Append(mlContext.Regression.Trainers.Sdca());
            
            var model = pipeline.Fit(dataView);

            return mlContext.Model.CreatePredictionEngine<SecValData, SecValDataPrediction>(model);
        }

        /************************
         *
         *
         */
        static PredictionEngine<SecValData, SecValDataPrediction> Prediction_OnlineGradientDescent(IDataView dataView) {

            var pipeline = mlContext
                    .Transforms
                    .CopyColumns(outputColumnName: "Label", inputColumnName:"Value")
                    .Append(mlContext.Transforms.Concatenate("Features", "Seconds"))
                    .Append(mlContext.Regression.Trainers.OnlineGradientDescent());
            
            var model = pipeline.Fit(dataView);

            return mlContext.Model.CreatePredictionEngine<SecValData, SecValDataPrediction>(model);
        }

        /************************
         *
         *
         */
        static void doPrediction(PredictionAlg alg, IDataView dataView, int second, int len) {

            PredictionEngine<SecValData,SecValDataPrediction> 
                predictionFunction;

            switch (alg)  {
                case PredictionAlg.FastTree:
                    predictionFunction = Prediction_FastTree(dataView);
                    break;
                case PredictionAlg.LbfgsPoisson:
                    predictionFunction = Prediction_LbfgsPoisson(dataView);
                    break;
                case PredictionAlg.Gam:
                    predictionFunction = Prediction_Gam(dataView);
                    break;
                case PredictionAlg.Sdca:
                    predictionFunction = Prediction_Sdca(dataView);
                    break;
                case PredictionAlg.OnlineGradientDescent:
                    predictionFunction = Prediction_OnlineGradientDescent(dataView);
                    break;
                default:
                    predictionFunction = Prediction_Ols(dataView);
                    break;
            }

            // Write the results                     
			System.IO.StreamWriter file =  new System.IO.StreamWriter(OutFilePath); 

            for (int i = 0; i < len; i++) {
                var FutureData = new SecValData() { Seconds = second+i, Value = 0 };
                var prediction = predictionFunction.Predict(FutureData);            
                file.WriteLine(prediction.Result);
            }
#if false            
            for (int i = 0; i < second+len; i++) {
                var FutureData = new SecValData() { Seconds = i, Value = 0 };
                var prediction = predictionFunction.Predict(FutureData);            
                Console.WriteLine("{0} {1} ",FutureData.Seconds, prediction.Result);
            }
#endif            
		    file.Close();
        }

        //====================================================
        //
        // Anomaly Detection
        //
        //====================================================
        static void DetectSpike(IDataView dataView, int confidence, int valueHistoryLenght) {

	        // Setup the estimator arguments
            string outputColumnName = nameof(TSPrediction.Prediction);
            string inputColumnName = nameof(TSData.val);

            //STEP 1: Create Estimtator   
            var estimator = mlContext.Transforms.DetectIidSpike(
				outputColumnName, 
				inputColumnName,
				confidence: confidence, 
				pvalueHistoryLength: valueHistoryLenght);

            //STEP 2:The Transformed Model.
            //In IID Spike detection, we don't need to do training, we just need to do transformation. 
            //As you are not training the model, there is no need to load IDataView with real data, you just need schema of data.
            //So create empty data view and pass to Fit() method. 
            ITransformer tansformedModel = estimator.Fit(CreateEmptyDataView());

            //STEP 3: Use/test model
            //Apply data transformation to create predictions.
            IDataView transformedData = tansformedModel.Transform(dataView);
            var predictions = mlContext.Data.CreateEnumerable<TSPrediction>(transformedData, reuseRowObject: false);

            // Write the results                     
			System.IO.StreamWriter file =  new System.IO.StreamWriter(OutFilePath); 
            int count = 0;
			foreach (var p in predictions) {
                if (p.Prediction[0] == 1) { file.WriteLine(count); }
				count ++;
            }
		   file.Close();
        }

        //====================================================
        //
        //
        //
        //====================================================
        static void DetectSpikeBySsa(IDataView dataView, 
            int confidence, int valueHistoryLenght,
            int trainingWindowSize, int seasonalityWindowSize) {

	        // Setup the estimator arguments
            string outputColumnName = nameof(TSPrediction.Prediction);
            string inputColumnName = nameof(TSData.val);

            //STEP 1: Create Estimtator   
            var estimator = mlContext.Transforms.DetectSpikeBySsa(
				outputColumnName, 
				inputColumnName,
				confidence: confidence, 
				pvalueHistoryLength: valueHistoryLenght,
                trainingWindowSize: trainingWindowSize,
                seasonalityWindowSize: seasonalityWindowSize);
            //Console.WriteLine("{0}\t{1:0.00}\t{2:0.00}\t{3:0.00}  <-- alert is on, predicted changepoint", p.Prediction[0], p.Prediction[1], p.Prediction[2], p.Prediction[3]);
            //Console.WriteLine("{0}",trainingWindowSize);

            //STEP 2:The Transformed Model.
            //In IID Spike detection, we don't need to do training, we just need to do transformation. 
            //As you are not training the model, there is no need to load IDataView with real data, you just need schema of data.
            //So create empty data view and pass to Fit() method. 
            ITransformer tansformedModel = estimator.Fit(dataView);
            //Console.WriteLine("Row Count: {0}",dataView.GetRowCount());

            //STEP 3: Use/test model
            //Apply data transformation to create predictions.
            IDataView transformedData = tansformedModel.Transform(dataView);
            
            var predictions = mlContext.Data.CreateEnumerable<TSPrediction>(transformedData, reuseRowObject: false);

            // Write the results                     
			System.IO.StreamWriter file =  new System.IO.StreamWriter(OutFilePath); 
            int count = 0;
			foreach (var p in predictions) {
                if (p.Prediction[0] == 1) { file.WriteLine(count); }
				count ++;
            }
		   file.Close();
        }


        //====================================================
        //
        //
        //
        //====================================================
        static void DetectAnomalyBySrCnn(IDataView dataView,
                int windowSize, int backAddWindowSize,
                int lookaheadWindowSize, int averageingWindowSize,
                int judgementWindowSize, double threshold) {

            // Setup the estimator arguments
            string outputColumnName = nameof(TSPrediction.Prediction);
            string inputColumnName = nameof(TSData.val);

            //STEP 1: Create Esimtator   
            var estimator = mlContext.Transforms.DetectAnomalyBySrCnn(
                    outputColumnName, inputColumnName, 
                    windowSize, backAddWindowSize, lookaheadWindowSize, 
                    averageingWindowSize, judgementWindowSize, threshold);

            //STEP 2:The Transformed Model. 
            ITransformer tansformedModel = estimator.Fit(CreateEmptyDataView());

            //STEP 3: Use/test model
            //Apply data transformation to create predictions.
            IDataView transformedData = tansformedModel.Transform(dataView);
            var predictions = mlContext.Data.CreateEnumerable<TSPrediction>(transformedData, reuseRowObject: false);
                        
            //Console.WriteLine("Alert\tScore\tP-Value");
            System.IO.StreamWriter file =  new System.IO.StreamWriter("ts-out.txt"); 
            int count = 0;
            foreach (var p in predictions) {
                if (p.Prediction[0] == 1)  { file.WriteLine(count); }
                count ++;
            }
            file.Close();
        }
/* */


        static void DetectChangepoint(int size, IDataView dataView)
        {
          Console.WriteLine("===============Detect Persistent changes in pattern===============");

	    // Setup the estimator arguments
            string outputColumnName = nameof(TSPrediction.Prediction);
            string inputColumnName = nameof(TSData.val);


          //STEP 1: Setup transformations using DetectIidChangePoint
          var estimator = mlContext.Transforms.DetectIidChangePoint(outputColumnName, inputColumnName, confidence: 95, changeHistoryLength: size / 4);

          //STEP 2:The Transformed Model.
          //In IID Change point detection, we don't need need to do training, we just need to do transformation. 
          //As you are not training the model, there is no need to load IDataView with real data, you just need schema of data.
          //So create empty data view and pass to Fit() method. 
          ITransformer tansformedModel = estimator.Fit(CreateEmptyDataView());

          //STEP 3: Use/test model
          //Apply data transformation to create predictions.
          IDataView transformedData = tansformedModel.Transform(dataView);
          var predictions = mlContext.Data.CreateEnumerable<TSPrediction>(transformedData, reuseRowObject: false);
                       
          Console.WriteLine($"{nameof(TSPrediction.Prediction)} column obtained post-transformation.");
          Console.WriteLine("Alert\tScore\tP-Value\tMartingale value");
            
          int count = 1;
          foreach(var p in predictions)
          {
	         Console.Write(count);Console.Write(" ");
	     count ++;
             if (p.Prediction[0] == 1)
             {
                 Console.WriteLine("{0}\t{1:0.00}\t{2:0.00}\t{3:0.00}  <-- alert is on, predicted changepoint", p.Prediction[0], p.Prediction[1], p.Prediction[2], p.Prediction[3]);
             }
             else
             { 
                 Console.WriteLine("{0}\t{1:0.00}\t{2:0.00}\t{3:0.00}",  p.Prediction[0], p.Prediction[1], p.Prediction[2], p.Prediction[3]);                  
             }            
          }
          Console.WriteLine("");
        }

        public static string GetAbsolutePath(string relativePath)
        {
            FileInfo _dataRoot = new FileInfo(typeof(Program).Assembly.Location);
            string assemblyFolderPath = _dataRoot.Directory.FullName;

            string fullPath = Path.Combine(assemblyFolderPath, relativePath);

            return fullPath;
        }

        private static IDataView CreateEmptyDataView()
        {
            //Create empty DataView. We just need the schema to call fit()
            IEnumerable<TSData> enumerableData = new List<TSData>();
            var dv = mlContext.Data.LoadFromEnumerable(enumerableData);
            return dv;
        }
    }
}
