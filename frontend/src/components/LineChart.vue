<template>
    <div>
        <Line :data="chartData" :options="chartOptions" />
    </div>
</template>

<script setup>
import { Line } from 'vue-chartjs'
import {
    Chart as ChartJS,
    Title,
    Tooltip,
    Legend,
    LineElement,
    CategoryScale,
    LinearScale,
    PointElement,
    TimeScale // Import TimeScale for the time axis
} from 'chart.js'
import 'chartjs-adapter-date-fns' // Import adapter for date functionality

// Register Chart.js components
ChartJS.register(Title, Tooltip, Legend, LineElement, CategoryScale, LinearScale, PointElement, TimeScale)

const props = defineProps({
    chartData: {
        type: Object,
        required: true
    },
    chartOptions: {
        type: Object,
        required: false,
        default: () => ({})
    }
})
</script>

<style scoped>
.line-chart-container {
    width: 100%;
    height: 100%;
}
</style>