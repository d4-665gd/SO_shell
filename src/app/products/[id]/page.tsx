'use client';
import { useEffect, useState } from 'react';
import { useParams } from 'next/navigation';
import { useDispatch, useSelector } from 'react-redux';
import { addFavorite, removeFavorite} from '../../../store/favoritesSlice';
import { RootState } from '../../../store/store';

export default function ProductDetailPage() {
  const { id } = useParams();
  const dispatch = useDispatch();
  const [product, setProduct] = useState<any>(null);
  const favorites = useSelector((state: RootState)=>
    state.favorites.items);

  useEffect(() => {
    if (id) {
      fetch(`https://rickandmortyapi.com/api/character/${id}`)
        .then(res => res.json())
        .then(data => setProduct(data));
    }
  }, [id]);

  if (!product) return <div>Loading...</div>;

  //checar producto si esta en favoritos
     const isFavorite = favorites.some(item => item.id === product.id);

     //funcionamiento del corazon
     const handleToggleFavorite = () => {
       if(isFavorite){
         dispatch(removeFavorite(product.id));
       } else {
        dispatch(addFavorite(product));
       }
     };

     return (
      <div>
        <h1>{product.name}</h1>
        <img src={product.image} alt={product.name} width={200} />
        {/* Corazón rojo si es favorito, blanco si no */}
        <button onClick={handleToggleFavorite} style={{ fontSize: '2rem', background: 'none', border: 'none', cursor: 'pointer' }}>
        Agregar a favoritos  {isFavorite ? '❤️' : '🤍'}
        </button>
        {/* Si quieres dejar el botón clásico de texto, puedes quitarlo */}
        {/* <button onClick={() => dispatch(addFavorite(product))}>Add to Favorites</button> */}
      </div>
    );
}