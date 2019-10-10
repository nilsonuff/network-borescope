using Microsoft.ML.Data;

namespace DataStructures
{
	public class TSData
    {
        [LoadColumn(0)]
        public float val;
    }
	
	public class TSPrediction
    {
        //vector to hold alert,score,p-value values
        [VectorType(3)]
        public double[] Prediction { get; set; }
    }

    public class SecValData
    {
        [LoadColumn(0)]
        public float  Seconds;

        [LoadColumn(1)]
        public float Value;
    }

    public class SecValDataPrediction
    {
        [ColumnName("Score")]
        public float Result;
    }
}
