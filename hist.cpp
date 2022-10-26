//
// Created by sao on 10/25/22.
//

#include "hist.h"

#include <png.h>
#include <cmath>
#include <iostream>
#include <ascdm.h>
#include <gsl/gsl_histogram2d.h>

void scale_histogram(gsl_histogram2d* hist, const std::string& scale) {
    double max_val = gsl_histogram2d_max_val(hist);

    if (scale == "linear")
        gsl_histogram2d_scale(hist, 255. / max_val);

    if (scale == "log") {
        const double a = 10000.;
        const double s1 = a / max_val;
        const double s2 = 255. / log(a);
        auto *bin = hist->bin;
        for (long i = 0; i < 8192 * 8192; i++)
            bin[i] = s2 * log1p(s1 * bin[i]);
    }
}

void create_png(const std::string& output, png_bytepp row_pointers) {
    // Create file output
    FILE* fp = fopen(output.c_str(), "wb");
    // Create structure for write
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    // Create info structure
    png_infop info = png_create_info_struct(png);
    // Initialize file output
    png_init_io(png, fp);
    // Set image properties
    png_set_IHDR(
            png,
            info,
            8192,
            8192,
            8,
            PNG_COLOR_TYPE_RGB,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
    );
    // Write png information
    png_write_info(png, info);

    // Write png data
    if (setjmp(png_jmpbuf(png)))
        std::cerr << "[write_png_file] Error during writing bytes\n";
    png_write_image(png, row_pointers);
    if (setjmp(png_jmpbuf(png)))
        std::cerr << "[write_png_file] Error during writing bytes\n";
    png_write_end(png, nullptr);
    if (fp)
        fclose(fp);
}

png_bytepp malloc_png_ptr() {
    auto row_pointers = (png_bytepp) malloc(sizeof(png_bytep) * 8192);
    for (long i = 0; i < 8192; i++) {
        row_pointers[i] = (png_bytep) malloc(8192 * 3);
    }
    return row_pointers;
}

void free_png_ptr(png_bytepp row_pointers) {
    for (long i = 0; i < 8192; i++) {
        if (row_pointers[i])
            free(row_pointers[i]);
    }
    if (row_pointers)
        free(row_pointers);
}

void calc_acis_histogram(const std::string& evt3_path, const std::string& output, const std::string& scale) {
    // Open the EVT3 file.
    dmDataset* ds = dmDatasetOpen(evt3_path.c_str());
    dmBlock* regions_table = dmBlockOpen(ds, "EVENTS");
    long n_rows = dmTableGetNoRows(regions_table);
    auto* x_column = dmTableOpenColumn(regions_table, "x");
    auto* y_column = dmTableOpenColumn(regions_table, "y");
    auto* energy_column = dmTableOpenColumn(regions_table, "energy");

    float tlmin, tlmax;
    dmDescriptorGetRange_f(y_column, &tlmin, &tlmax);
    long side_length = static_cast<long>(tlmax - tlmin);

    // Initialize the histogram
    gsl_histogram2d* r_hist = gsl_histogram2d_alloc(side_length, side_length);
    gsl_histogram2d* g_hist = gsl_histogram2d_alloc(side_length, side_length);
    gsl_histogram2d* b_hist = gsl_histogram2d_alloc(side_length, side_length);

    gsl_histogram2d_set_ranges_uniform(r_hist,
                                       tlmin, tlmax,
                                       tlmin, tlmax);
    gsl_histogram2d_set_ranges_uniform(g_hist,
                                       tlmin, tlmax,
                                       tlmin, tlmax);
    gsl_histogram2d_set_ranges_uniform(b_hist,
                                       tlmin, tlmax,
                                       tlmin, tlmax);

    for (long row = 0; row < n_rows; row++) {
        double x = dmGetScalar_f(x_column);
        double y = dmGetScalar_f(y_column);
        float energy = dmGetScalar_f(energy_column);

        if (energy > 200 && energy < 1500)
            gsl_histogram2d_accumulate(r_hist, x, y, 1);
        if (energy >= 1500 && energy < 2000)
            gsl_histogram2d_accumulate(g_hist, x, y, 1);
        if (energy >= 2000 && energy <= 8000)
            gsl_histogram2d_accumulate(b_hist, x, y, 1);

        dmTableNextRow(regions_table);
    }

    // Clean up block and dataset, we don't need it anymore
    dmBlockClose(regions_table);
    dmDatasetClose(ds);

    scale_histogram(r_hist, scale);
    scale_histogram(g_hist, scale);
    scale_histogram(b_hist, scale);

    // Initialize
    auto row_pointers = malloc_png_ptr();

    // Fill image
    for (long w = 0; w < 8192; w++) {
        png_bytep row = row_pointers[w];
        for (long h = 0; h < 8192; h++) {
            png_bytep ptr = &(row[h * 3]);
            ptr[0] = static_cast<png_byte>(gsl_histogram2d_get(r_hist, w, h));
            ptr[1] = static_cast<png_byte>(gsl_histogram2d_get(g_hist, w, h));
            ptr[2] = static_cast<png_byte>(gsl_histogram2d_get(b_hist, w, h));
        }
    }

    create_png(output, row_pointers);

    // Cleanup GSL
    gsl_histogram2d_free(r_hist);
    gsl_histogram2d_free(g_hist);
    gsl_histogram2d_free(b_hist);

    // Cleanup PNG
    free_png_ptr(row_pointers);
}

void calc_hrc_histogram(const std::string& evt3_path, const std::string& output, const std::string& scale) {
    // Open the EVT3 file.
    dmDataset* ds = dmDatasetOpen(evt3_path.c_str());
    dmBlock* regions_table = dmBlockOpen(ds, "EVENTS");
    long n_rows = dmTableGetNoRows(regions_table);
    auto* x_column = dmTableOpenColumn(regions_table, "x");
    auto* y_column = dmTableOpenColumn(regions_table, "y");

    float tlmin = 0, tlmax = 0;
    dmDescriptorGetRange_f(y_column, &tlmin, &tlmax);
    long side_length = static_cast<long>(tlmax - tlmin);
    if (tlmax == 0) {
        std::cerr << "Error tlmax is zero. Using .5-32768.5\n";
        tlmin = .5; tlmax = 32768.5; side_length = 32768;
    }

    gsl_histogram2d* hist = gsl_histogram2d_alloc(side_length, side_length);
    gsl_histogram2d_set_ranges_uniform(hist,
                                       tlmin, tlmax,
                                       tlmin, tlmax);

    for (long row = 0; row < n_rows; row++) {
        double x = dmGetScalar_f(x_column);
        double y = dmGetScalar_f(y_column);

        gsl_histogram2d_accumulate(hist, x, y, 1);

        dmTableNextRow(regions_table);
    }

    // Clean up block and dataset, we don't need it anymore
    dmBlockClose(regions_table);
    dmDatasetClose(ds);

    scale_histogram(hist, scale);

    // Initialize
    auto row_pointers = malloc_png_ptr();

    // Fill image
    for (long w = 0; w < 8192; w++) {
        png_bytep row = row_pointers[w];
        for (long h = 0; h < 8192; h++) {
            png_bytep ptr = &(row[h * 3]);
            auto byte = static_cast<png_byte>(gsl_histogram2d_get(hist, w, h));
            ptr[0] = ptr[1] = ptr[2] = byte;
        }
    }

    create_png(output, row_pointers);

    // Cleanup GSL
    gsl_histogram2d_free(hist);

    // Cleanup PNG
    free_png_ptr(row_pointers);
}

void calc_histogram(const std::string& evt3_path, const std::string& output, const std::string& scale) {
    if (evt3_path.find("acis") != std::string::npos)
        calc_acis_histogram(evt3_path, output, scale);
    else
        calc_hrc_histogram(evt3_path, output, scale);
}