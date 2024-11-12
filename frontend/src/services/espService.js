import axios from 'axios';

// Set the ESP32 IP address
const ESP32_BASE_URL = 'http://172.20.10.5/data';

// Function to fetch data from ESP32
// export async function fetchEspData() {
//     try {
//         const response = await axios.get(ESP32_BASE_URL);
//         return response.data;
//     } catch (error) {
//         console.error("Failed to fetch data from ESP32:", error);
//         throw error;
//     }
// }

// write a mock function which generates random data

export async function fetchEspData() {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve({
                lux: parseFloat((Math.random() * 10000).toFixed(2)),
                smoothedLux: parseFloat((Math.random() * 10000).toFixed(2)),
                luxChange: parseFloat((Math.random() * 50).toFixed(2)),
                adjustedSpeed: parseFloat((Math.random() * 500).toFixed(2)),
                adjustedAcceleration: parseFloat((Math.random() * 200).toFixed(2)),
                targetPosition: Math.floor(Math.random() * 2048),
                currentPosition: Math.floor(Math.random() * 2048)
            })
        }, 1000) // Simulate network delay
    })
}
