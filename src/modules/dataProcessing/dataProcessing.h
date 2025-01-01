enum dataProcessing_state {
    DATA_PROCESS_IDLE,
    DATA_PROCESS_FILE,
    DATA_PROCESS_PARSING
};

dataProcessing_state getDataProcessingState() {
    return DATA_PROCESS_IDLE;
};