#include <pulse/simple.h>
#include <pulse/error.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#define BUFSIZE 1024
#define PRIORITY_VOLUME_REDUCTION 0.2 // Константа X

struct AudioSource {
    pa_simple *stream;
    int priority;

    AudioSource(pa_simple *s, int p) : stream(s), priority(p) {}
};

bool comparePriority(const AudioSource &a, const AudioSource &b) {
    return a.priority > b.priority;
}

int main() {
    std::vector<AudioSource> audioSources;
    pa_sample_spec ss;
    ss.format = PA_SAMPLE_S16LE;
    ss.channels = 1;
    ss.rate = 44100;

    // Создаем аудиоисточник с наивысшим приоритетом (микрофон)
    pa_simple *microphoneStream = pa_simple_new(nullptr, "Microphone", PA_STREAM_RECORD, nullptr, "record", &ss, nullptr, nullptr, nullptr);

    if (!microphoneStream) {
        std::cerr << "Error: pa_simple_new (microphone) failed." << std::endl;
        return -1;
    }

    audioSources.emplace_back(microphoneStream, INT_MAX); // Максимальный приоритет

    // Добавляем произвольные аудиоисточники с произвольными приоритетами
    pa_simple *source1 = pa_simple_new(nullptr, "Source1", PA_STREAM_RECORD, nullptr, "record", &ss, nullptr, nullptr, nullptr);
    pa_simple *source2 = pa_simple_new(nullptr, "Source2", PA_STREAM_RECORD, nullptr, "record", &ss, nullptr, nullptr, nullptr);

    if (!source1 || !source2) {
        std::cerr << "Error: pa_simple_new (source) failed." << std::endl;
        pa_simple_free(microphoneStream);
        pa_simple_free(source1);
        pa_simple_free(source2);
        return -1;
    }

    audioSources.emplace_back(source1, 10); // Произвольный приоритет
    audioSources.emplace_back(source2, 5);  // Произвольный приоритет

    // Открываем поток воспроизведения
    pa_simple *outputStream = pa_simple_new(nullptr, "PriorityAudioOut", PA_STREAM_PLAYBACK, nullptr, "playback", &ss, nullptr, nullptr, nullptr);

    if (!outputStream) {
        std::cerr << "Error: pa_simple_new (output) failed." << std::endl;
        for (auto &audioSource : audioSources) {
            pa_simple_free(audioSource.stream);
        }
        return -1;
    }

    std::cout << "Start capturing and processing audio. Press Ctrl+C to stop." << std::endl;

    while (true) {
        // Сортируем источники по приоритету
        std::sort(audioSources.begin(), audioSources.end(), comparePriority);

        int16_t mixedBuf[BUFSIZE] = {0};

        // Считываем данные из каждого приоритетного источника и смешиваем их
        for (const auto &audioSource : audioSources) {
            int16_t buf[BUFSIZE];

            if (pa_simple_read(audioSource.stream, buf, sizeof(buf), nullptr) < 0) {
                std::cerr << "Error: pa_simple_read failed." << std::endl;
                break;
            }

            // Смешиваем данные
            for (int i = 0; i < BUFSIZE; ++i) {
                mixedBuf[i] += buf[i];
            }
        }

        // Управление громкостью
        if (audioSources.size() > 1) {
            // Если есть более одного источника, уменьшаем громкость остальных
            for (size_t i = 1; i < audioSources.size(); ++i) {
                for (int i = 0; i < BUFSIZE; ++i) {
                    mixedBuf[i] = static_cast<int16_t>(mixedBuf[i] * (1.0 - PRIORITY_VOLUME_REDUCTION));
                }
            }
        }

        // Воспроизводим смешанные данные
        if (pa_simple_write(outputStream, mixedBuf, sizeof(mixedBuf), nullptr) < 0) {
            std::cerr << "Error: pa_simple_write failed." << std::endl;
            break;
        }
    }

    // Освобождаем ресурсы
    for (auto &audioSource : audioSources) {
        pa_simple_free(audioSource.stream);
    }

    if (outputStream) {
        pa_simple_free(outputStream);
    }

    return 0;
}


// "Эхо": звук с микрофона сразу выводится на динамик
// Каналов с другими источниками нет и приоритета тоже нет

//#include <pulse/simple.h>
//#include <pulse/error.h>
//#include <iostream>
//
//#define BUFSIZE 1024
//
//int main() {
//    // Создание структур для входного и выходного звуковых потоков
//    pa_sample_spec ss;
//    ss.format = PA_SAMPLE_S16LE;
//    ss.channels = 1;
//    ss.rate = 44100;
//
//    pa_simple *input_stream = nullptr;
//    pa_simple *output_stream = nullptr;
//    int error;
//
//    // Открытие звукового потока для ввода (микрофона)
//    if (!(input_stream = pa_simple_new(nullptr, "Mic to Speaker", PA_STREAM_RECORD, nullptr, "record", &ss, nullptr, nullptr, &error))) {
//        std::cerr << "Error: pa_simple_new (input) failed: " << pa_strerror(error) << std::endl;
//        return -1;
//    }
//
//    // Открытие звукового потока для вывода (динамика)
//    if (!(output_stream = pa_simple_new(nullptr, "Mic to Speaker", PA_STREAM_PLAYBACK, nullptr, "playback", &ss, nullptr, nullptr, &error))) {
//        std::cerr << "Error: pa_simple_new (output) failed: " << pa_strerror(error) << std::endl;
//        pa_simple_free(input_stream);
//        return -1;
//    }
//
//    std::cout << "Start capturing from microphone and playing to speaker. Press Ctrl+C to stop." << std::endl;
//
//    while (true) {
//        int16_t buf[BUFSIZE];
//
//        // Чтение данных с микрофона
//        if (pa_simple_read(input_stream, buf, sizeof(buf), &error) < 0) {
//            std::cerr << "Error: pa_simple_read failed: " << pa_strerror(error) << std::endl;
//            break;
//        }
//
//        // Воспроизведение данных через динамик
//        if (pa_simple_write(output_stream, buf, sizeof(buf), &error) < 0) {
//            std::cerr << "Error: pa_simple_write failed: " << pa_strerror(error) << std::endl;
//            break;
//        }
//    }
//
//    // Закрытие звуковых потоков
//    if (input_stream)
//        pa_simple_free(input_stream);
//
//    if (output_stream)
//        pa_simple_free(output_stream);
//
//    return 0;
//}
