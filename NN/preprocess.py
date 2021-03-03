import librosa
import os
from sklearn.model_selection import train_test_split
from keras.utils import to_categorical
import numpy as np
from tqdm import tqdm
import scipy.fftpack as fft

DATA_PATH = "./data/"
N_FFT = 1024
N_MELS = 30
N_COEFS = 21

# Input: Folder Path
# Output: Tuple (Label, Indices of the labels, one-hot encoded labels)
def get_labels(path=DATA_PATH):
    labels = os.listdir(path)
    label_indices = np.arange(0, len(labels))
    return labels, label_indices, to_categorical(label_indices)


# Handy function to convert wav2mfcc
def wav2mfccOld(file_path, max_len=15):
    wave, sr = librosa.load(file_path, mono=True, sr=None)
    
    #wave = wave[::3]
    #mfcc = librosa.feature.mfcc(wave, sr=16000)
    mfcc = librosa.feature.melspectrogram(wave, sr=16000, n_mels=30, n_fft=1024, hop_length=1024, center=False, htk=False)
    #mfcc = librosa.feature.mfcc(wave, sr=16000, fmax=8000, n_mels=30, n_mfcc=30, n_fft=1024, hop_length=1024)
    #mfcc = abs(mfcc)
    mfcc = mfcc / mfcc.max()
    #mfcc = librosa.power_to_db(mfcc, top_db=80.0)

    # If maximum length exceeds mfcc lengths then pad the remaining ones
    if (max_len > mfcc.shape[1]):
        pad_width = max_len - mfcc.shape[1]
        mfcc = np.pad(mfcc, pad_width=((0, 0), (0, pad_width)), mode='constant')

    # Else cutoff the remaining parts
    else:
        mfcc = mfcc[:, :max_len]
    
    return mfcc

def wavffc(file_path):
    wave, sr = librosa.load(file_path, mono=True, sr=None)
    fft_out = fft.fft(wave, n=N_FFT, axis=0)[:N_FFT // 2 + 1]
    S_pwr = np.abs(fft_out)**2
    #S_pwr = S_pwr / S_pwr.max()
    return S_pwr

def create_col(y):
    assert y.shape == (N_FFT,)

    # Create time-series window
    #fft_window = librosa.filters.get_window('hann', N_FFT, fftbins=True)
    #assert fft_window.shape == (1024,), fft_window.shape

    # Hann window
    #y_windowed = fft_window * y
    #assert y_windowed.shape == (1024,), y_windowed.shape

    # FFT
    fft_out = fft.fft(y, axis=0)[:N_FFT // 2 + 1]
    assert fft_out.shape == (N_FFT // 2 + 1,), fft_out.shape

    # Power spectrum
    S_pwr = np.abs(fft_out)**2

    assert S_pwr.shape == (N_FFT // 2 + 1,)

    # Generation of Mel Filter Banks
    #mel_basis = librosa.filters.mel(16000, n_fft=1024, n_mels=30, htk=False)
    #assert mel_basis.shape == (30, 513)

    # Apply Mel Filter Banks
    #S_mel = np.dot(mel_basis, S_pwr)
    #S_mel.astype(np.float32)
    S_mel = librosa.feature.melspectrogram(S=S_pwr, sr=16000, n_mels=30, n_fft=N_FFT, hop_length=N_FFT, center=False, htk=False)
    assert S_mel.shape == (30,)

    return S_mel

def wav2mfcc(file_path, max_len=15):
    wave, sr = librosa.load(file_path, mono=True, sr=None)
    frames = wave[:15360]
    #noise = white_noise(1, 16000, len(frames))
    #frames = frames + noise
    #frames = librosa.util.frame(frames, frame_length=N_FFT, hop_length=N_FFT // 2)
    wave = np.reshape(frames, (N_FFT, max_len), order='F')
    assert wave.shape == (N_FFT, max_len)

    S_mel = np.empty((30, max_len), dtype=np.float32, order='C')
    for col_index in range(0, max_len):
        S_mel[:, col_index] = create_col(wave[:, col_index])

    S_mel = S_mel[1:N_COEFS + 1, :]
    # Scale according to reference power
    S_mel = S_mel / S_mel.max()
    # Convert to dB
    S_mel = librosa.power_to_db(S_mel, top_db=80.0)
    #assert S_log_mel.shape == (30, 32)
    assert S_mel.shape == (N_COEFS, max_len)
    
    return S_mel

def save_data_to_array(path=DATA_PATH, max_len=15):
    labels, _, _ = get_labels(path)

    for label in labels:
        # Init mfcc vectors
        mfcc_vectors = []

        wavfiles = [path + label + '/' + wavfile for wavfile in os.listdir(path + '/' + label)]
        for wavfile in tqdm(wavfiles, "Saving vectors of label - '{}'".format(label)):
            mfcc = wav2mfcc(wavfile, max_len=max_len)
            mfcc_vectors.append(mfcc)
        np.save(label + '.npy', mfcc_vectors)


def get_train_test(split_ratio=0.6, random_state=42):
    # Get available labels
    labels, indices, _ = get_labels(DATA_PATH)

    # Getting first arrays
    X = np.load(labels[0] + '.npy')
    y = np.zeros(X.shape[0])

    # Append all of the dataset into one single array, same goes for y
    for i, label in enumerate(labels[1:]):
        x = np.load(label + '.npy')
        X = np.vstack((X, x))
        y = np.append(y, np.full(x.shape[0], fill_value= (i + 1)))

    assert X.shape[0] == len(y)

    return train_test_split(X, y, test_size= (1 - split_ratio), random_state=random_state, shuffle=True)



def prepare_dataset(path=DATA_PATH):
    labels, _, _ = get_labels(path)
    data = {}
    for label in labels:
        data[label] = {}
        data[label]['path'] = [path  + label + '/' + wavfile for wavfile in os.listdir(path + '/' + label)]

        vectors = []

        for wavfile in data[label]['path']:
            mfcc = wav2mfcc(wavfile, max_len=15)
            #wave, sr = librosa.load(wavfile, mono=True, sr=None)
            # Downsampling
            #wave = wave[::3]
            #mfcc = librosa.feature.mfcc(wave, sr=16000, n_mels=22, n_mfcc=30, fmax=8000, n_fft=1024, hop_length=1024)
            #librosa.util.normalize(mfcc, axis=1)
            #mfcc = librosa.power_to_db(mfcc, top_db=80.0)
            #mfcc = librosa.feature.mfcc(wave, sr=16000)
            vectors.append(mfcc)

        data[label]['mfcc'] = vectors

    return data


def load_dataset(path=DATA_PATH):
    data = prepare_dataset(path)

    dataset = []

    for key in data:
        for mfcc in data[key]['mfcc']:
            dataset.append((key, mfcc))

    return dataset[:100]


# print(prepare_dataset(DATA_PATH))

