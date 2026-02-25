# TSP Solver Docker Image
# Multi-stage build for minimal final image

# Stage 1: Build
FROM gcc:15 AS builder

WORKDIR /src

# Copy source files
COPY . .

# Build miniAI with optimizations
RUN make clean && \
    make && \
    strip TSP_COMPARISON

# Stage 2: Runtime
FROM ubuntu:24.04

LABEL org.opencontainers.image.title="TSP Solver"
LABEL org.opencontainers.image.description="Exact, heuristic, and meta-heuristic algorithms for the TSP, implemented in pure C."
LABEL org.opencontainers.image.authors="Nelson Ramos"
LABEL org.opencontainers.image.source="https://github.com/nelsonramosua/miniAI"

# Install minimal runtime dependencies
RUN apt-get update && \
    apt-get upgrade -y && \
    rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy binary and required files from builder
COPY --from=builder /src/TSP_COMPARISON .
COPY --from=builder /src/graphs ./graphs
COPY --from=builder /src/README.md .
COPY --from=builder /src/LICENSE .

# Set permissions
RUN chmod +x TSP_COMPARISON

# Create non-root user
RUN useradd -m tspuser && \
    chown -R tspuser:tspuser /app
USER tspuser

# Default: run all graphs. Override with: docker run tsp N=3
ENTRYPOINT ["./TSP_COMPARISON"]
CMD ["help"]