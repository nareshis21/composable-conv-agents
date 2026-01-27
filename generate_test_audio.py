import asyncio
import edge_tts
import json
import os
import subprocess

# Standard Whisper format: 16kHz, mono, s16le
TARGET_SAMPLERATE = 16000

async def generate_audio():
    # 1. Load suite
    with open("automated_test_suite.json", "r") as f:
        suite = json.load(f)

    # 2. Setup directory
    output_dir = "test_audio"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    print(f"🎬 Starting Audio Generation in '{output_dir}'")
    
    # Choose a neutral "Golden Speaker" (US Male/Female)
    VOICE = "en-US-GuyNeural" 

    for category, utterances in suite.items():
        print(f"\n📂 Processing Category: {category}")
        cat_dir = os.path.join(output_dir, category)
        if not os.path.exists(cat_dir):
            os.makedirs(cat_dir)

        for i, text in enumerate(utterances):
            # Filename-safe text
            safe_name = "".join([c if c.isalnum() else "_" for c in text[:20]]).lower()
            temp_mp3 = os.path.join(cat_dir, f"{i}_{safe_name}.mp3")
            final_wav = os.path.join(cat_dir, f"{i}_{safe_name}.wav")

            print(f"   [{i+1}/{len(utterances)}] Generating: {text}")

            # Generate MP3 via Edge TTS
            communicate = edge_tts.Communicate(text, VOICE)
            await communicate.save(temp_mp3)

            # Convert to 16kHz Mono WAV using FFmpeg
            # We use subprocess to call ffmpeg (which user has via ffplay/ffmpeg tools)
            try:
                subprocess.run([
                    "ffmpeg", "-y", "-i", temp_mp3, 
                    "-ar", str(TARGET_SAMPLERATE), 
                    "-ac", "1", 
                    "-acodec", "pcm_s16le", 
                    final_wav
                ], check=True, capture_output=True)
                # Cleanup temp mp3
                os.remove(temp_mp3)
            except Exception as e:
                print(f"   ❌ Error converting {text}: {e}")

    print("\n✅ All 40 WAV files generated successfully!")

if __name__ == "__main__":
    asyncio.run(generate_audio())
