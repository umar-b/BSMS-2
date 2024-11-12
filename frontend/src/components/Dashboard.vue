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
            <div class="line-chart-container h-80"> <!-- Increased height to 320px -->
              <LineChart :chartData="luxChartData.value" :chartOptions="chartOptions" />
              <h2 class="text-lg font-semibold text-center mt-2">Current Lux</h2>
            </div>

            <div class="line-chart-container h-80">
              <LineChart :chartData="smoothedLuxChartData.value" :chartOptions="chartOptions" />
              <h2 class="text-lg font-semibold text-center mt-2">Smoothed Lux</h2>
            </div>

            <div class="line-chart-container h-80">
              <LineChart :chartData="luxChangeChartData.value" :chartOptions="chartOptions" />
              <h2 class="text-lg font-semibold text-center mt-2">Lux Change</h2>
            </div>

            <div class="line-chart-container h-80">
              <LineChart :chartData="adjustedSpeedChartData.value" :chartOptions="chartOptions" />
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
import { fetchEspData } from '~/services/espService'
import LineChart from '~/components/LineChart.vue' // Ensure correct path

// State variables for loading and error handling
const loading = ref(true)
const error = ref(null)
const sensorStatus = ref("Offline")
const lastUpdate = ref("")
const statusClass = computed(() => (sensorStatus.value === "Online" ? "text-green-400" : "text-red-400"))

// Chart options for each line chart
const chartOptions = {
  responsive: true,
  maintainAspectRatio: false,
  scales: {
    x: {
      type: 'time',
      time: {
        unit: 'second',
        tooltipFormat: 'HH:mm:ss'
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
  },
  plugins: {
    legend: {
      display: true
    }
  }
}

// Define maxDataPoints outside of loadData
const maxDataPoints = 50

// Initialize chart data using `ref`
const luxChartData = ref({
  labels: [],
  datasets: [{ label: 'Current Lux', data: [], borderColor: '#f87979', fill: false }]
})

const smoothedLuxChartData = ref({
  labels: [],
  datasets: [{ label: 'Smoothed Lux', data: [], borderColor: '#36a2eb', fill: false }]
})

const luxChangeChartData = ref({
  labels: [],
  datasets: [{ label: 'Lux Change', data: [], borderColor: '#ffce56', fill: false }]
})

const adjustedSpeedChartData = ref({
  labels: [],
  datasets: [{ label: 'Adjusted Speed', data: [], borderColor: '#4bc0c0', fill: false }]
})

// Function to update chart data
function updateChart(chartDataRef, label, value) {
  const chartData = chartDataRef.value
  if (!chartData || !chartData.labels || !chartData.datasets) {
    console.error('Invalid chartData:', chartData)
    return
  }

  if (chartData.labels.length >= maxDataPoints) {
    chartData.labels.shift()
    chartData.datasets[0].data.shift()
  }
  chartData.labels.push(label)
  chartData.datasets[0].data.push(value)
}

// Function to fetch and update data with a maximum limit to prevent overflow
async function loadData() {
  try {
    const result = await fetchEspData()
    const currentTime = new Date() // Use Date object for better compatibility

    // Update each chart’s data
    updateChart(luxChartData, currentTime, result.lux)
    updateChart(smoothedLuxChartData, currentTime, result.smoothedLux)
    updateChart(luxChangeChartData, currentTime, result.luxChange)
    updateChart(adjustedSpeedChartData, currentTime, result.adjustedSpeed)

    // Update sidebar info
    lastUpdate.value = currentTime.toLocaleTimeString()
    sensorStatus.value = 'Online'
    error.value = null
  } catch (err) {
    console.error('Network Error:', err)
    error.value = 'Network Error: Failed to fetch data from ESP32'
    sensorStatus.value = 'Offline'
  } finally {
    loading.value = false
  }
}

// Fetch data on mount and set up interval for continuous updates
onMounted(() => {
  loadData()
  setInterval(loadData, 5000)
})
</script>

<style scoped>
.dashboard-container {
  width: 100vw;
  height: 100vh;
}

.line-chart-container {
  height: 320px; /* Increased height for better visibility */
}
</style>
