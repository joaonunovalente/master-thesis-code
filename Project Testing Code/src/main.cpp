#include <Arduino.h>
#include <vector>

// Function to find all peak values and their indices in a vector
std::vector<std::pair<double, int>> findPeaks(const std::vector<double> &data)
{
    std::vector<std::pair<double, int>> peaks; // Initialize a vector to store peak value and index pairs

    for (int i = 1; i < data.size() - 1; i++)
    {
        if (data[i] >= data[i - 1] && data[i] >= data[i + 1])
        {
            // The current element is a peak
            peaks.push_back(std::make_pair(data[i], i));
        }
    }

    return peaks;
}

void setup()
{
    Serial.begin(115200);

    std::vector<double> data = {10.0, 5.0, 8.0, 20.0, 15.0, 12.0, 18.0, 6.0, 3.0, 6.0, 9.0, 10.0, 11.0, 6.0, 4.0};

    // Call the findPeaks function with the vector as input
    std::vector<std::pair<double, int>> peakInfo = findPeaks(data);

    // Output all the peak values and their indices
    for (const auto &peak : peakInfo)
    {
        double peakValue = peak.first;
        int peakIndex = peak.second;
        Serial.println("Peak value: " + String(peakValue));
        Serial.println("Peak index: " + String(peakIndex));
    }
}

void loop()
{
    // Delay for a while before repeating
    delay(1000);
}
