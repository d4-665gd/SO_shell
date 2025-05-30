'use client';
import { useSelector, useDispatch } from 'react-redux';
import { RootState } from '../../store/store';
import { removeFavorite } from '../../store/favoritesSlice';

export default function FavoritesPage() {
  const favorites = useSelector((state: RootState) => state.favorites.items);
  const dispatch = useDispatch();

  if (favorites.length === 0) {
    return <p>No favorites yet.</p>;
  }

  return (
    <div>
      <h1>Your Favorites</h1>
      <ul>
        {favorites.map(product => (
          <li key={product.id}>
            <img src={product.image} alt={product.name} width={50} />
            <span>{product.name}</span>
            {/* Corazón rojo como botón para eliminar */}
            <button
              onClick={() => dispatch(removeFavorite(product.id))}
              style={{ fontSize: '1.5rem', background: 'none', border: 'none', cursor: 'pointer' }}
              aria-label="Remove from favorites"
            >
              ❤️
            </button>
          </li>
        ))}
      </ul>
    </div>
  );
}