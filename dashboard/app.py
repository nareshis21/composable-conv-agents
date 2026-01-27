from flask import Flask, jsonify, send_from_directory
from flask_cors import CORS
import pandas as pd
import os

app = Flask(__name__)
CORS(app)

CSV_PATH = os.path.join(os.path.dirname(__file__), "..", "benchmark_results.csv")

@app.route('/api/metrics')
def get_metrics():
    if not os.path.exists(CSV_PATH):
        return jsonify({"error": "CSV not found", "data": []})
    
    try:
        df = pd.read_csv(CSV_PATH)
        # Handle cases where CSV is empty or being written
        if df.empty:
            return jsonify({"data": [], "stats": {}})
        
        # Calculate some P-values
        stats = {
            "p50_e2e": float(df['Total_E2E'].median()),
            "p90_e2e": float(df['Total_E2E'].quantile(0.9)),
            "avg_asr": float(df['ASR_Latency'].mean()),
            "avg_llm": float(df['LLM_TTFT'].mean()),
            "total_tokens": int(df['Tokens'].sum()),
            "count": int(len(df))
        }
        
        return jsonify({
            "data": df.to_dict(orient='records'),
            "stats": stats
        })
    except Exception as e:
        return jsonify({"error": str(e), "data": []})

@app.route('/')
def index():
    return send_from_directory('static', 'index.html')

if __name__ == '__main__':
    app.run(port=5000, debug=True)
