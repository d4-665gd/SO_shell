import { configureStore } from '@reduxjs/toolkit';
import favoritesReducer from './favoritesSlice';

const loadFromLocalStorage = () => {
  try {
    const serializedState = localStorage.getItem('favorites');
    if (serializedState === null) return undefined;
    return { favorites: JSON.parse(serializedState) };
  } catch (e) {
    return undefined;
  }
};

const saveToLocalStorage = (state: any) => {
  try {
    const serializedState = JSON.stringify(state.favorites);
    localStorage.setItem('favorites', serializedState);
  } catch (e) {
    // Ignore write errors
  }
};

export const store = configureStore({
  reducer: {
    favorites: favoritesReducer,
  },
  preloadedState: loadFromLocalStorage(),
});

store.subscribe(() => {
    saveToLocalStorage(store.getState());
  });


export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;
