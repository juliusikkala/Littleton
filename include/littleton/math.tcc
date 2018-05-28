namespace lt
{

template<typename T>
void cholesky_decomposition(T* matrix, unsigned n)
{
    for(unsigned i = 0; i < n; ++i)
    {
        for(unsigned j = 0; j <= i; ++j)
        {
            T sum = T(0);
            for(unsigned k = 0; k < j; ++k)
                sum += matrix[i*n+k]*matrix[j*n+k];
            T& a = matrix[i*n + j];
            if(i == j) a = glm::sqrt(a - sum);
            else
            {
                a = (a - sum) / matrix[j*n+j];
                matrix[j*n + i] = a;
            }
        }
    }
}

template<typename T>
void forward_substitution(const T* matrix, const T* b, T* x, unsigned n)
{
    for(unsigned i = 0; i < n; ++i)
    {
        T sum = T(0);
        for(unsigned j = 0; j < i; ++j)
            sum += matrix[i*n + j] * x[j];
        x[i] = (b[i] - sum)/matrix[i*n + i];
    }
}

template<typename T>
void backward_substitution(const T* matrix, const T* b, T* x, unsigned n)
{
    for(int i = n-1; i >= 0; --i)
    {
        T sum = T(0);
        for(int j = n-1; j > i; --j)
            sum += matrix[i*n + j] * x[j];
        x[i] = (b[i] - sum)/matrix[i*n + i];
    }
}

template<typename T>
void transpose(const T* matrix, unsigned n)
{
    for(int i = 0; i < n; ++i)
    {
        for(int j = 0; j < n; ++j)
        {
            T& a = matrix[i*n + j];
            T& b = matrix[j*n + i];
            T tmp = a;
            a = b;
            b = tmp;
        }
    }
}

template<typename T>
void matrix_transpose_product(
    const T* matrix,
    unsigned n,
    unsigned m,
    T* product
){
    for(int i = 0; i < m; ++i)
    {
        for(int j = 0; j <= i; ++j)
        {
            float& sum = product[i*m + j] = T(0);
            for(int k = 0; k < n; ++k)
                sum += matrix[k*m + i] * matrix[k*m + j];
            product[j*m + i] = sum;
        }
    }
}

template<typename T>
void matrix_transpose_vector_product(
    const T* matrix,
    unsigned n,
    unsigned m,
    const T* vector,
    T* product
){
    for(int i = 0; i < m; ++i)
    {
        float& sum = product[i] = T(0);
        for(int j = 0; j < n; ++j)
            sum += matrix[j*m + i] * vector[j];
    }
}

template<typename F, typename S, typename T, typename U>
std::vector<T> fit_linear_least_squares(
    F&& f,
    S* approx_args,
    unsigned num_approximators,
    U* data_locations,
    T* data_values,
    unsigned data_points
){
    // Populate design matrix
    std::vector<T> X(num_approximators * data_points, T(1));
    for(unsigned i = 0; i < data_points; ++i)
    {
        for(unsigned j = 0; j < num_approximators; ++j)
        {
            X[i*num_approximators + j] = f(approx_args[j], data_locations[i]);
        }
    }

    std::vector<T> R(num_approximators * num_approximators, T(0));
    std::vector<T> b(num_approximators, T(0));
    std::vector<T> z(num_approximators, T(0));

    matrix_transpose_product(X.data(), data_points, num_approximators, R.data());
    cholesky_decomposition(R.data(), num_approximators);
    matrix_transpose_vector_product(
        X.data(), data_points, num_approximators, data_values, b.data()
    );
    forward_substitution(R.data(), b.data(), z.data(), num_approximators);
    backward_substitution(R.data(), z.data(), b.data(), num_approximators);
    return b;
}

}
