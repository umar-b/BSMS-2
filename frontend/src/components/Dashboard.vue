<template>
    <div class="dashboard-container h-screen bg-gray-900 text-gray-200 p-6">
        <div class="grid grid-cols-4 gap-6 h-full">
            <!-- Sidebar for widgets (1/4 width) -->
            <div class="col-span-4 md:col-span-1 bg-gray-800 rounded-lg p-4 h-full">
                <h2 class="text-xl font-semibold mb-4">Information & Widgets</h2>

                <!-- Example widgets -->
                <div class="widget mb-6 p-4 bg-gray-700 rounded">
                    <h3 class="text-lg font-semibold">Sensor Status</h3>
                    <p>Status: <span :class="statusClass">{{ sensorStatus }}</span></p>
                </div>

                <div class="widget mb-6 p-4 bg-gray-700 rounded">
                    <h3 class="text-lg font-semibold">Last Update</h3>
                    <p>{{ lastUpdate }}</p>
                </div>

                <div class="widget p-4 bg-gray-700 rounded">
                    <h3 class="text-lg font-semibold">Custom Widget</h3>
                    <p>Put additional info here.</p>
                </div>
            </div>

            <!-- Main content area for charts (3/4 width) -->
            <div class="col-span-4 md:col-span-3 bg-gray-800 rounded-lg p-6 h-full overflow-y-auto">
                <h1 class="text-2xl font-bold mb-6">ESP32 Sensor Dashboard</h1>

                <!-- Display error message -->
                <div v-if="error" class="p-4 mb-4 bg-red-600 text-white rounded">
                    Error: {{ error }}
                </div>

                <div v-if="loading" class="text-gray-400">
                    Loading data...
                </div>

                <div v-else>
                    <!-- Stacked charts -->
                    <div class="space-y-6">
                        <div class="line-chart-container h-60">
                            <LineChart :chartData="charts.lux.chartData" :chartOptions="chartOptions" />
                            <h2 class="text-lg font-semibold text-center mt-2">Current Lux</h2>
                        </div>

                        <div class="line-chart-container h-60">
                            <LineChart :chartData="charts.smoothedLux.chartData" :chartOptions="chartOptions" />
                            <h2 class="text-lg font-semibold text-center mt-2">Smoothed Lux</h2>
                        </div>

                        <div class="line-chart-container h-60">
                            <LineChart :chartData="charts.luxChange.chartData" :chartOptions="chartOptions" />
                            <h2 class="text-lg font-semibold text-center mt-2">Lux Change</h2>
                        </div>

                        <div class="line-chart-container h-60">
                            <LineChart :chartData="charts.adjustedSpeed.chartData" :chartOptions="chartOptions" />
                            <h2 class="text-lg font-semibold text-center mt-2">Adjusted Speed</h2>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { fetchEspData } from '../espService'

// State variables for loading and error handling
const loading = ref(true)
const error = ref(null)

// Status and update information for sidebar
const sensorStatus = ref("Offline") // Start with "Offline"
const lastUpdate = ref("")

// Status class for sensor (changing color based on status)
const statusClass = computed(() => (sensorStatus.value === "Online" ? "text-green-400" : "text-red-400"))

// Chart options for each line chart
const chartOptions = {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
        x: {
            type: 'time',
            time: {
                unit: 'second'
            },
            title: {
                display: true,
                text: 'Time'
            }
        },
        y: {
            beginAtZero: true,
            title: {
                display: true,
                text: 'Value'
            }
        }
    }
}

// Chart data structure for various metrics
const charts = ref({
    lux: { chartData: { labels: [], datasets: [{ label: 'Current Lux', data: [] }] } },
    smoothedLux: { chartData: { labels: [], datasets: [{ label: 'Smoothed Lux', data: [] }] } },
    luxChange: { chartData: { labels: [], datasets: [{ label: 'Lux Change', data: [] }] } },
    adjustedSpeed: { chartData: { labels: [], datasets: [{ label: 'Adjusted Speed', data: [] }] } }
})

// Function to fetch and update data
async function loadData() {
    try {
        const result = await fetchEspData()
        const currentTime = new Date()

        // Update each chart's data
        charts.value.lux.chartData.labels.push(currentTime)
        charts.value.lux.chartData.datasets[0].data.push(result.lux)

        charts.value.smoothedLux.chartData.labels.push(currentTime)
        charts.value.smoothedLux.chartData.datasets[0].data.push(result.smoothedLux)

        charts.value.luxChange.chartData.labels.push(currentTime)
        charts.value.luxChange.chartData.datasets[0].data.push(result.luxChange)

        charts.value.adjustedSpeed.chartData.labels.push(currentTime)
        charts.value.adjustedSpeed.chartData.datasets[0].data.push(result.adjustedSpeed)

        lastUpdate.value = currentTime.toLocaleTimeString()
        sensorStatus.value = "Online"
        error.value = null
    } catch (err) {
        console.error("Network Error:", err)
        error.value = 'Network Error: Failed to fetch data from ESP32'
        sensorStatus.value = "Offline" // Set to Offline if fetching fails
    } finally {
        loading.value = false
    }
}


// Fetch data on mount and set up interval for continuous updates
onMounted(() => {
    loadData()
    setInterval(loadData, 5000) // Update data every 5 seconds
})
</script>

<style scoped>
/* Full-width layout adjustments */
.dashboard-container {
    width: 100vw;
    height: 100vh;
    margin: 0;
}

.line-chart-container {
    height: 240px;
}
</style>