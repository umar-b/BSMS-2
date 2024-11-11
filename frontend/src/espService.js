import axios from 'axios';

// Set the ESP32 IP address
const ESP32_BASE_URL = 'http://172.20.10.5/data';

// Function to fetch data from ESP32
export async function fetchEspData() {
    try {
        const response = await axios.get(ESP32_BASE_URL);
        return response.data;
    } catch (error) {
        console.error("Failed to fetch data from ESP32:", error);
        throw error;
    }
}
