'use client';
import Link from 'next/link';
import { useEffect, useState } from 'react';

interface Product {
  id: number;
  name: string;
  image: string; 
}

export default function ProductsPage() {
  const [products, setProducts] = useState<Product[]>([]);

  useEffect(() => {
    fetch('https://rickandmortyapi.com/api/character')
      .then(res => res.json())
      .then(data => setProducts(data.results));
  }, []);

  return (
    <div>
      <h1>Products</h1>
      <ul>
      {products.map((product) =>
  product.id && product.name ? (
    <li key={product.id}>
    <Link href={`/products/${product.id}`} key={product.id}>
      {product.name}
    </Link>
               </li>
  ) : null
)}
      </ul>
    </div>
  );
}