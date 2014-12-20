#include "EvolutionSpinner.h"

EvolutionSpinner::EvolutionSpinner(QObject* parent)
	: QObject(parent)
{
	++m_gen;
}
